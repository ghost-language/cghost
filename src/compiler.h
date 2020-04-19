#ifndef ghost_compiler_h
#define ghost_compiler_h

// This module defines the compiler for Ghost. It takes a string of source code
// and lexes, parses, and compiles it. Ghost uses a single-pass compiler. It
// does not build an actual AST during parsing and then consume that to
// generate code. Instead, the parser directly emits bytecode.
//
// This forces a few restrictions on the grammar and semantics of the language.
// Things like forward references and arbitrary lookahead are much harder. We
// get a lot in return for that, though.
//
// The implementation is much simpler since we don't need to define a bunch of
// AST data structures. More so, we don't have to deal with managing memory for
// AST objects. The compiler does almost no dynamic allocation while running.
//
// Compilation is also faster since we don't create a bunch of temporary data
// structures and destroy them after generating code.

#include "object.h"
#include "vm.h"

bool compile(const char* source, Chunk* chunk);

#endif