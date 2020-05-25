#ifndef ghost_h
#define ghost_h

#include <stdlib.h>

typedef struct GhostVM GhostVM;

// A generic allocation that handles all explicit memory management used by
// Ghost. It's used like so:
//
// - To allocate new memory, [memory] is NULL and [oldSize] is zero.
//
// - To attempt to grow an existing allocation, [memory] is the memory,
//   [oldSize] is its previous size, and [newSize] is the desired size.
//    It returns [memory] if it was able to grow it in place, or a new pointer
//    if it had to move it.
//
// - To shrink memory, [memory], [oldSize], and [newSize] are the same as above
//   but it will always return [memory]. If [newSize] is zero, the memory will
//   be freed and `NULL` will be returned.
//
// - To free memory, [newSize] will be zero.
typedef void *(*GhostReallocateFn)(void *memory, size_t oldSize, size_t newSize);

// Create a new Ghost virtual machine. It allocates memory for the VM itself
// using [reallocateFn] and then uses that throughout its lifetime to manage
// memory.
GhostVM* ghostNewVM(GhostReallocateFn reallocateFn);

// Disposes of all resources to use by [vm], which was previously created by a
// call to [ghostNewVM].
void ghostFreeVM(GhostVM* vm);

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

// InterpretResult interpret(GhostVM *vm, const char *source);

// Runs [source], a string of Ghost source code in [vm]. Returns zero if
// sucessful.
InterpretResult ghostInterpret(GhostVM *vm, const char *source);

#endif