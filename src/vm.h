#ifndef ghost_vm_h
#define ghost_vm_h

// This defines the virtual machine that executes instructions. Ghost's virtual
// machine is one part of its internal architecture. You hand it a chunk of
// code--literally a chunk--and it runs it.
//
// The main entry point into the VM is through the interpret() method. The VM
// runs the chunk and then responds with an interpresation result, indicating
// if the code was successful or encountered any compile or runtime errors.

#include "object.h"
#include "table.h"
#include "chunk.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;
    Table strings;
    ObjUpvalue* openUpvalues;

    Obj* objects;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

void defineNative(const char *name, NativeFn function);
void defineNativeVoid(const char *name, NativeVoidFn function);

void runtimeError(const char *format, ...);

#endif