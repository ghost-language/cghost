#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "include/ghost.h"
#include "memory.h"
#include "vm.h"

#if DEBUG_LOG_GC
    #include <stdio.h>
    #include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

void* reallocate(GhostVM *vm, void* previous, size_t oldSize, size_t newSize) {
    vm->bytesAllocated += newSize - oldSize;

    if (newSize > oldSize) {
        #if DEBUG_STRESS_GC
            collectGarbage();
        #endif
    }

    if (vm->bytesAllocated > vm->nextGC) {
        collectGarbage(vm);
    }

    if (newSize == 0) {
        free(previous);

        return NULL;
    }

    return realloc(previous, newSize);
}

void markObject(GhostVM *vm, Obj* object) {
    if (object == NULL) return;
    if (object->isMarked) return;

    #if DEBUG_LOG_GC
        printf("%p mark ", (void*)object);
        printValue(OBJ_VAL(object));
        printf("\n");
    #endif

    object->isMarked = true;

    if (vm->grayCapacity < vm->grayCount + 1) {
        vm->grayCapacity = GROW_CAPACITY(vm->grayCapacity);
        vm->grayStack = realloc(vm->grayStack, sizeof(Obj*) * vm->grayCapacity);
    }

    vm->grayStack[vm->grayCount++] = object;
}

void markValue(GhostVM *vm, Value value) {
    if (!IS_OBJ(value)) return;
    markObject(vm, AS_OBJ(value));
}

static void markArray(GhostVM *vm, ValueArray* array) {
    for (int i = 0; i < array->count; i++) {
        markValue(vm, array->values[i]);
    }
}

static void blackenObject(GhostVM *vm, Obj* object) {
    #if DEBUG_LOG_GC
        printf("%p blacken ", (void*)object);
        printValue(OBJ_VAL(object));
        printf("\n");
    #endif

    switch (object->type) {
        case OBJ_BOUND_METHOD: {
            ObjBoundMethod* bound = (ObjBoundMethod*)object;
            markValue(vm, bound->receiver);
            markObject(vm, (Obj*)bound->method);
            break;
        }

        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*)object;
            markObject(vm, (Obj*)klass->name);
            markTable(vm, &klass->methods);
            break;
        }

        case OBJ_NATIVE_CLASS: {
            ObjNativeClass *klass = (ObjNativeClass*)object;
            markObject(vm, (Obj*)klass->name);
            markTable(vm, &klass->methods);
            break;
        }

        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            markObject(vm, (Obj*)closure->function);

            for (int i = 0; i < closure->upvalueCount; i++) {
                markObject(vm, (Obj*)closure->upvalues[i]);
            }
            break;
        }

        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            markObject(vm, (Obj*)function->name);
            markArray(vm, &function->chunk.constants);
            break;
        }

        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)object;
            markObject(vm, (Obj*)instance->klass);
            markTable(vm, &instance->fields);
            break;
        }

        case OBJ_UPVALUE:
            markValue(vm, ((ObjUpvalue*)object)->closed);
            break;

        case OBJ_NATIVE:
        case OBJ_STRING:
        case OBJ_LIST:
            break;
    }
}

static void freeObject(GhostVM* vm, Obj* object) {
    #if DEBUG_LOG_GC
        printf("%p free type %d\n", (void*)object, object->type);
    #endif

    switch (object->type) {
        case OBJ_BOUND_METHOD:
            FREE(vm, ObjBoundMethod, object);
            break;
        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*)object;
            freeTable(vm, &klass->methods);
            FREE(vm, ObjClass, object);
            break;
        }

        case OBJ_NATIVE_CLASS: {
            ObjNativeClass* klass = (ObjNativeClass*)object;
            freeTable(vm, &klass->methods);
            FREE(vm, ObjClass, object);
            break;
        }

        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            FREE_ARRAY(vm, ObjUpvalue*, closure->upvalues, closure->upvalueCount);
            FREE(vm, ObjClosure, object);
            break;
        }

        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            freeChunk(vm, &function->chunk);
            FREE(vm, ObjFunction, object);
            break;
        }

        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)object;
            freeTable(vm, &instance->fields);
            FREE(vm, ObjInstance, object);
            break;
        }

        case OBJ_NATIVE: {
            FREE(vm, ObjNative, object);
            break;
        }

        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;

            FREE_ARRAY(vm, char, string->chars, string->length + 1);
            FREE(vm, ObjString, object);

            break;
        }

        case OBJ_UPVALUE: {
            FREE(vm, ObjUpvalue, object);
            break;
        }

        case OBJ_LIST: {
            break;
        }
    }
}

static void markRoots(GhostVM *vm) {
    for(Value* slot = vm->stack; slot < vm->stackTop; slot++) {
        markValue(vm, *slot);
    }

    for (int i = 0; i < vm->frameCount; i++) {
        markObject(vm, (Obj*)vm->frames[i].closure);
    }

    for (ObjUpvalue* upvalue = vm->openUpvalues; upvalue != NULL; upvalue = upvalue->next) {
        markObject(vm, (Obj*)upvalue);
    }

    markTable(vm, &vm->globals);
    markCompilerRoots();
    markObject(vm, (Obj*)vm->constructorString);
}

static void traceReferences(GhostVM *vm) {
    while (vm->grayCount > 0) {
        Obj* object = vm->grayStack[--vm->grayCount];
        blackenObject(vm, object);
    }
}

static void sweep(GhostVM *vm) {
    Obj* previous = NULL;
    Obj* object = vm->objects;

    while (object != NULL) {
        if (object->isMarked) {
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            Obj* unreached = object;
            object = object->next;

            if (previous != NULL) {
                previous->next = object;
            } else {
                vm->objects = object;
            }

            freeObject(vm, unreached);
        }
    }
}

void collectGarbage(GhostVM *vm) {
    #if DEBUG_LOG_GC
        printf("-- gc begin\n");
        size_t before = vm.bytesAllocated;
    #endif

    markRoots(vm);
    traceReferences(vm);
    tableRemoveWhite(&vm->strings);
    sweep(vm);

    vm->nextGC = vm->bytesAllocated * GC_HEAP_GROW_FACTOR;

    #if DEBUG_LOG_GC
            printf("-- gc end\n");
            printf("   collected %ld bytes (from %ld to %ld) next at %ld\n", before - vm->bytesAllocated, before, vm->bytesAllocated, vm->nextGC);
    #endif
}

void freeObjects(GhostVM *vm) {
    Obj* object = vm->objects;

    while (object != NULL) {
        Obj* next = object->next;
        freeObject(vm, object);
        object = next;
    }

    free(vm->grayStack);
}