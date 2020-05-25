#ifndef ghost_memory_h
#define ghost_memory_h

#include "include/ghost.h"
#include "object.h"

#define ALLOCATE(vm, type, count) \
    (type*)reallocate(vm, NULL, 0, sizeof(type) * (count))

#define FREE(vm, type, pointer) \
    reallocate(vm, pointer, sizeof(type), 0)

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(vm, previous, type, oldCount, count) \
    (type*)reallocate(vm, previous, sizeof(type) * (oldCount), \
        sizeof(type) * (count))

#define FREE_ARRAY(vm, type, pointer, oldCount) \
    reallocate(vm, pointer, sizeof(type) * (oldCount), 0)

void* reallocate(GhostVM *vm, void* previous, size_t oldSize, size_t newSize);
void markObject(GhostVM *vm, Obj* object);
void markValue(GhostVM *vm, Value value);
void collectGarbage(GhostVM *vm);
void freeObjects(GhostVM *vm);

#endif