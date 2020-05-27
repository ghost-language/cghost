#ifndef ghost_modules_h
#define ghost_modules_h

#include "../include/ghost.h"
#include "../vm.h"
#include "assert.h"
#include "math.h"

void defineNativeMethod(GhostVM *vm, ObjNativeClass *klass, const char *name, NativeFn function);

#endif