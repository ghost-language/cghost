#ifndef ghost_modules_h
#define ghost_modules_h

#include "../vm.h"
#include "math.h"

void defineNativeMethod(ObjNativeClass *klass, const char *name, NativeFn function);

#endif