#ifndef ghost_vm_h
#define ghost_vm_h

// This defines the virtual machine that executes instructions. Ghost's virtual
// machine is one part of its internal architecture. You hand it a chunk of
// code--literally a chunk--and it runs it.
//
// The main entry point into the VM is through the interpret() method. The VM
// runs the chunk and then responds with an interpresation result, indicating
// if the code was successful or encountered any compile or runtime errors.

#include "chunk.h"
#include "object.h"
#include "table.h"
#include "value.h"

#include "include/ghost.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;

struct GhostVM {
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;
    Table strings;
    ObjString* constructorString;
    ObjUpvalue* openUpvalues;

    // Garbage collection bookkeeping
    size_t bytesAllocated;
    size_t nextGC;

    Obj* objects;
    int grayCount;
    int grayCapacity;
    Obj** grayStack;
};

void push(GhostVM *vm, Value value);
Value pop(GhostVM *vm);

void defineNative(GhostVM *vm, const char *name, NativeFn function);

void runtimeError(GhostVM *vm, const char *format, ...);
bool isFalsey(Value value);

#endif