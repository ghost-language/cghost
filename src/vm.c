#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "include/ghost.h"
#include "object.h"
#include "memory.h"
#include "modules/modules.h"
#include "native.h"
#include "datatypes/string.h"
#include "datatypes/list.h"
#include "utilities.h"
#include "vm.h"
#include "modules/math.h"

static void resetStack(GhostVM *vm) {
    vm->stackTop = vm->stack;
    vm->frameCount = 0;
    vm->openUpvalues = NULL;
}

void runtimeError(GhostVM *vm, const char* format, ...) {
    fputs("Error: ", stderr);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fputs("\n", stderr);

    // Print stack trace
    for (int i = vm->frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm->frames[i];
        ObjFunction* function = frame->closure->function;

        // -1 because the IP is sitting on the next instruction to be
        // executed

        size_t instruction = frame->ip - function->chunk.code - 1;

        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);

        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    resetStack(vm);
}

void defineNative(GhostVM *vm, const char* name, NativeFn function) {
    push(vm, OBJ_VAL(copyString(vm, name, (int)strlen(name))));
    push(vm, OBJ_VAL(newNative(vm, function)));
    tableSet(vm, &vm->globals, AS_STRING(vm->stack[0]), vm->stack[1]);
    pop(vm);
    pop(vm);
}

GhostVM *ghostNewVM(GhostReallocateFn reallocateFn) {
    GhostVM* vm = reallocateFn(NULL, 0, sizeof(GhostVM));

    resetStack(vm);
    vm->objects = NULL;

    vm->bytesAllocated = 0;
    vm->nextGC = 1024 * 1024;

    vm->grayCount = 0;
    vm->grayCapacity = 0;
    vm->grayStack = NULL;

    initTable(&vm->globals);
    initTable(&vm->strings);

    vm->constructorString = NULL;
    vm->constructorString = copyString(vm, "constructor", 11);

    defineAllNatives(vm);
    registerMathModule(vm);

    return vm;
}

void ghostFreeVM(GhostVM *vm) {
    freeTable(vm, &vm->globals);
    freeTable(vm, &vm->strings);

    vm->constructorString = NULL;

    freeObjects(vm);

    free(vm);
}

void push(GhostVM *vm, Value value) {
    *vm->stackTop = value;
    vm->stackTop++;
}

Value pop(GhostVM *vm) {
    vm->stackTop--;

    return *vm->stackTop;
}

static Value peek(GhostVM *vm, int distance) {
    return vm->stackTop[-1 - distance];
}

static bool call(GhostVM *vm, ObjClosure* closure, int argCount) {
    if (argCount != closure->function->arity) {
        runtimeError(vm, "Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }

    if (vm->frameCount == FRAMES_MAX) {
        runtimeError(vm, "Stack overflow.");
        return false;
    }

    CallFrame* frame = &vm->frames[vm->frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;

    frame->slots = vm->stackTop - argCount - 1;
    return true;
}

static bool callValue(GhostVM *vm, Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
                vm->stackTop[-argCount - 1] = bound->receiver;
                return call(vm, bound->method, argCount);
            }

            case OBJ_CLASS: {
                ObjClass* klass = AS_CLASS(callee);
                vm->stackTop[-argCount - 1] = OBJ_VAL(newInstance(vm, klass));

                Value constructor;
                if (tableGet(&klass->methods, vm->constructorString, &constructor)) {
                    return call(vm, AS_CLOSURE(constructor), argCount);
                } else if (argCount != 0) {
                    runtimeError(vm, "Expected 0 arguments but got %d.", argCount);
                    return false;
                }

                return true;
            }

            case OBJ_CLOSURE: {
                return call(vm, AS_CLOSURE(callee), argCount);
            }

            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(vm, argCount, vm->stackTop - argCount);
                vm->stackTop -= argCount + 1;
                push(vm, result);
                return true;
            }

            default:
                // Non-callable object type
                break;
        }
    }

    runtimeError(vm, "Can only call functions and classes.");
    return false;
}

static bool invokeFromClass(GhostVM *vm, ObjClass* klass, ObjString* name, int argCount) {
    Value method;

    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError(vm, "Undefined property '%s'.", name->chars);
        return false;
    }

    return call(vm, AS_CLOSURE(method), argCount);
}

static bool invoke(GhostVM *vm, ObjString* name, int argCount) {
    Value receiver = peek(vm, argCount);

    if (!IS_OBJ(receiver)) {
        runtimeError(vm, "Can only invoke on objects.");
        return false;
    }

    switch (getObjType(receiver)) {
        case OBJ_NATIVE_CLASS: {
            ObjNativeClass *instance = AS_NATIVE_CLASS(receiver);

            Value function;

            if (!tableGet(&instance->methods, name, &function)) {
                runtimeError(vm, "Undefined property '%s'.", name->chars);
                return false;
            }

            return callValue(vm, function, argCount);
        }

        case OBJ_INSTANCE: {
            ObjInstance* instance = AS_INSTANCE(receiver);

            Value value;

            if (tableGet(&instance->fields, name, &value)) {
                vm->stackTop[-argCount - 1] = value;

                return callValue(vm, value, argCount);
            }

            return invokeFromClass(vm, instance->klass, name, argCount);
        }

        case OBJ_STRING: {
            return declareString(vm, name->chars, argCount + 1);
        }

        case OBJ_LIST: {
            return declareList(vm, name->chars, argCount + 1);
        }

         default: {
             runtimeError(vm, "Only instances have methods.");
             return false;
         }
    }
}

static bool bindMethod(GhostVM *vm, ObjClass* klass, ObjString* name) {
    Value method;

    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError(vm, "Undefined property '%s' on '%s'.", name->chars, klass->name->chars);
        return false;
    }

    ObjBoundMethod* bound = newBoundMethod(vm, peek(vm, 0), AS_CLOSURE(method));
    pop(vm);
    push(vm, OBJ_VAL(bound));
    return true;
}

static ObjUpvalue* captureUpvalue(GhostVM *vm, Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm->openUpvalues;

    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) return upvalue;

    ObjUpvalue* createdUpvalue = newUpvalue(vm, local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm->openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(GhostVM *vm, Value* last) {
    while (vm->openUpvalues != NULL && vm->openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm->openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm->openUpvalues = upvalue->next;
    }
}

static void defineMethod(GhostVM *vm, ObjString* name) {
    Value method = peek(vm, 0);
    ObjClass* klass = AS_CLASS(peek(vm, 1));
    tableSet(vm, &klass->methods, name, method);
    pop(vm);
}

bool isFalsey(Value value) {
    return IS_NULL(value) ||
           (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate(GhostVM *vm) {
    ObjString* b = AS_STRING(peek(vm, 0));
    ObjString* a = AS_STRING(peek(vm, 1));

    int length = a->length + b->length;
    char* chars = ALLOCATE(vm, char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(vm, chars, length);
    pop(vm);
    pop(vm);
    push(vm, OBJ_VAL(result));
}

static InterpretResult run(GhostVM *vm) {
    CallFrame* frame = &vm->frames[vm->frameCount - 1];

    #define READ_BYTE() (*frame->ip++)
    #define READ_SHORT() \
        (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
    #define READ_CONSTANT() \
        (frame->closure->function->chunk.constants.values[READ_BYTE()])

    #define READ_STRING() AS_STRING(READ_CONSTANT())

    #define BINARY_OP(valueType, op) \
        do { \
            if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) { \
                runtimeError(vm, "Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            \
            double b = AS_NUMBER(pop(vm)); \
            double a = AS_NUMBER(pop(vm)); \
            push(vm, valueType(a op b)); \
        } while (false)

    for (;;) {
        #if DEBUG_TRACE_EXECUTION
            printf("          ");

            for (Value* slot = vm->stack; slot < vm->stackTop; slot++) {
                printf("[ ");
                printValue(*slot);
                printf(" ]");
            }

            printf("\n");

            disassembleInstruction(&frame->closure->function->chunk, (int)(frame->ip - frame->closure->function->chunk.code));
        #endif

        uint8_t instruction;

        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(vm, constant);
                break;
            }

            case OP_NULL: push(vm, NULL_VAL); break;
            case OP_TRUE: push(vm, BOOL_VAL(true)); break;
            case OP_FALSE: push(vm, BOOL_VAL(false)); break;
            case OP_POP: pop(vm); break;

            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(vm, frame->slots[slot]);
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(vm, 0);
                break;
            }

            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;

                if (!tableGet(&vm->globals, name, &value)) {
                    runtimeError(vm, "Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(vm, value);
                break;
            }

            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(vm, &vm->globals, name, peek(vm, 0));

                pop(vm);
                break;
            }

            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();

                if (tableSet(vm, &vm->globals, name, peek(vm, 0))) {
                    tableDelete(&vm->globals, name);
                    runtimeError(vm, "Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(vm, *frame->closure->upvalues[slot]->location);
                break;
            }

            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(vm, 0);
                break;
            }

            case OP_GET_PROPERTY: {
                if (!IS_INSTANCE(peek(vm, 0))) {
                    runtimeError(vm, "Only instance have properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(peek(vm, 0));
                ObjString* name = READ_STRING();

                Value value;

                if (tableGet(&instance->fields, name, &value)) {
                    pop(vm); // Instance
                    push(vm, value);
                    break;
                }

                if (!bindMethod(vm, instance->klass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_SET_PROPERTY: {
                if (!IS_INSTANCE(peek(vm, 1))) {
                    runtimeError(vm, "Only instance have properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(peek(vm, 1));
                tableSet(vm, &instance->fields, READ_STRING(), peek(vm, 0));

                Value value = pop(vm);
                pop(vm);
                push(vm, value);
                break;
            }

            case OP_GET_SUPER: {
                ObjString* name = READ_STRING();
                ObjClass* superclass = AS_CLASS(pop(vm));

                if (!bindMethod(vm, superclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_EQUAL: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD: {
                if (IS_STRING(peek(vm, 0)) && IS_STRING(peek(vm, 1))) {
                    concatenate(vm);
                } else if (IS_NUMBER(peek(vm, 0)) && IS_NUMBER(peek(vm, 1))) {
                    double b = AS_NUMBER(pop(vm));
                    double a = AS_NUMBER(pop(vm));
                    push(vm, NUMBER_VAL(a + b));
                } else {
                    runtimeError(vm, "Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;

            case OP_MODULO: {
                if (! IS_NUMBER(peek(vm, 0)) && ! IS_NUMBER(peek(vm, 1))) {
                    runtimeError(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                double b = AS_NUMBER(pop(vm));
                double a = AS_NUMBER(pop(vm));
                push(vm, NUMBER_VAL(fmod(a, b)));
                break;
            }

            case OP_NOT: {
                push(vm, BOOL_VAL(isFalsey(pop(vm))));
                break;
            }

            case OP_NEGATE: {
                if (!IS_NUMBER(peek(vm, 0))) {
                    runtimeError(vm, "Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(vm, NUMBER_VAL(-AS_NUMBER(pop(vm))));

                break;
            }

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(vm, 0))) frame->ip += offset;
                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }

            case OP_CALL: {
                int argCount = READ_BYTE();

                if (!callValue(vm, peek(vm, argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }

                frame = &vm->frames[vm->frameCount - 1];
                break;
            }

            case OP_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();

                if (!invoke(vm, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }

                frame = &vm->frames[vm->frameCount - 1];
                break;
            }

            case OP_SUPER_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                ObjClass* superclass = AS_CLASS(pop(vm));

                if (!invokeFromClass(vm, superclass, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }

                frame = &vm->frames[vm->frameCount - 1];
                break;
            }

            case OP_CLOSURE: {
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = newClosure(vm, function);
                push(vm, OBJ_VAL(closure));

                for (int i = 0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();

                    if (isLocal) {
                        closure->upvalues[i] = captureUpvalue(vm, frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }

            case OP_CLOSE_UPVALUE:
                closeUpvalues(vm, vm->stackTop - 1);
                pop(vm);
                break;

            case OP_RETURN: {
                Value result = pop(vm);

                closeUpvalues(vm, frame->slots);

                vm->frameCount--;

                if (vm->frameCount == 0) {
                    pop(vm);
                    return INTERPRET_OK;
                }

                vm->stackTop = frame->slots;
                push(vm, result);

                frame = &vm->frames[vm->frameCount - 1];
                break;
            }

            case OP_CLASS:
                push(vm, OBJ_VAL(newClass(vm, READ_STRING())));
                break;

            case OP_INHERIT: {
                Value superclass = peek(vm, 1);

                if (!IS_CLASS(superclass)) {
                    runtimeError(vm, "Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjClass* subclass = AS_CLASS(peek(vm, 0));
                tableAddAll(vm, &AS_CLASS(superclass)->methods, &subclass->methods);
                pop(vm);  // subclass
                break;
            }

            case OP_METHOD:
                defineMethod(vm, READ_STRING());
                break;

            case OP_INCLUDE: {
                ObjString *fileName = AS_STRING(pop(vm));
                char *source = readFile(fileName->chars);

                ObjFunction *function = ghostCompile(vm, source);
                if (function == NULL) return INTERPRET_COMPILE_ERROR;

                push(vm, OBJ_VAL(function));
                ObjClosure *closure = newClosure(vm, function);
                pop(vm);

                frame = &vm->frames[vm->frameCount++];
                frame->ip = closure->function->chunk.code;
                frame->closure = closure;
                frame->slots = vm->stackTop - 1;

                break;
            }

            case OP_NEW_LIST: {
                ObjList* list = newList(vm);
                push(vm, OBJ_VAL(list));
                break;
            }

            case OP_ADD_LIST: {
                Value addValue = pop(vm);
                Value listValue = pop(vm);

                ObjList* list = AS_LIST(listValue);
                writeValueArray(vm, &list->values, addValue);

                push(vm, OBJ_VAL(list));
                break;
            }

            case OP_SUBSCRIPT: {
                Value indexValue = pop(vm);
                Value listValue = pop(vm);

                if (!IS_NUMBER(indexValue)) {
                    runtimeError(vm, "List index must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjList *list = AS_LIST(listValue);
                int index = AS_NUMBER(indexValue);

                push(vm, list->values.values[index]);
                break;
            }

            case OP_SUBSCRIPT_ASSIGN: {
                Value assignValue = peek(vm, 0);
                Value indexValue = peek(vm, 1);
                Value listValue = peek(vm, 2);

                if (!IS_OBJ(listValue)) {
                    runtimeError(vm, "Can only subscript lists.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                if (!IS_NUMBER(indexValue)) {
                    runtimeError(vm, "List index must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjList *list = AS_LIST(listValue);
                int index = AS_NUMBER(indexValue);

                if (index >= 0 && index < list->values.count) {
                    list->values.values[index] = assignValue;
                    push(vm, NULL_VAL);
                } else {
                    pop(vm);
                    pop(vm);
                    pop(vm);

                    push(vm, NULL_VAL);

                    runtimeError(vm, "List index out of bounds.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_SHORT
    #undef READ_CONSTANT
    #undef READ_STRING
    #undef BINARY_OP
}

InterpretResult ghostInterpret(GhostVM *vm, const char* source) {
    ObjFunction* function = ghostCompile(vm, source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(vm, OBJ_VAL(function));
    ObjClosure* closure = newClosure(vm, function);
    pop(vm);
    push(vm, OBJ_VAL(closure));
    callValue(vm, OBJ_VAL(closure), 0);

    return run(vm);
}