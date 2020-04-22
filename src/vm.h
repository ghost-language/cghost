#ifndef ghost_vm_h
#define ghost_vm_h

// This defines the virtual machine that executes instructions. Ghost's virtual
// machine is one part of its internal architecture. You hand it a chunk of
// code--literally a chunk--and it runs it.
//
// The main entry point into the VM is through the interpret() method. The VM
// runs the chunk and then responds with an interpresation result, indicating
// if the code was successful or encountered any compile or runtime errors.

#include "table.h"
#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    Value stack[STACK_MAX];
    Value* stackTop;
    Table strings;

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

#endif