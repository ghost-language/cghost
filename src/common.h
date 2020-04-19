#ifndef ghost_common_h
#define ghost_common_h

// This header contains macros and defines used across the entire Ghost
// implementation. In particular, it contains "configuration" defines that
// control how Ghost works. Some of these are only used while hacking on
// debugging Ghost itself.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// These flags are useful for debuggin and hacking on Ghost itself. They are not
// intended to be used for production code. They default to off.

// Uncomment this to print out the compiled bytecode.
// #define DEBUG_PRINT_CODE

// Uncomment this to print out executions as they occur.
// #define DEBUG_TRACE_EXECUTION

#endif