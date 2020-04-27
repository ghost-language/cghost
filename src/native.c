#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "native.h"
#include "object.h"
#include "vm.h"

static Value clockNative(int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value mathAbsNative(int argCount, Value* args) {
    if (argCount == 0) {
        runtimeError("math_abs() expects exactly one argument.");
        return NIL_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("math_abs() expects a number argument.");
        return NIL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(abs((int)AS_NUMBER(number)));
}

static Value mathAcosNative(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("math_acos() expects exactly one argument.");
        return NIL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("math_abs() expects a number argument.");
        return NIL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(acos((int)AS_NUMBER(number)));
}

static void printNative(int argCount, Value *args) {
    if (argCount == 0) {
        printf("\n");
        return;
    }

    for (int i = 0; i < argCount; i++) {
        Value value = args[i];
        printValue(value);
        printf("\n");
    }

    return;
}

static void writeNative(int argCount, Value *args) {
    if (argCount == 0) {
        printf("%c", '\0');
        return;
    }

    for (int i = 0; i < argCount; i++) {
        Value value = args[i];
        printValue(value);
        printf("%c", '\0');
    }

    return;
}

const char* nativeNames[] = {
    "clock", "math_abs", "math_acos"
};

NativeFn nativeFunctions[] = {
    clockNative, mathAbsNative, mathAcosNative,
};

const char* nativeVoidNames[] = {
    "print", "write",
};

NativeVoidFn nativeVoidFunctions[] = {
    printNative, writeNative,
};

void defineAllNatives() {
    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); i++) {
        defineNative(nativeNames[i], nativeFunctions[i]);
    }

    for (uint8_t i = 0; i < sizeof(nativeVoidNames) / sizeof(nativeVoidNames[0]); i++) {
        defineNativeVoid(nativeVoidNames[i], nativeVoidFunctions[i]);
    }
}