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

static Value printNative(int argCount, Value *args) {
    if (argCount == 0) {
        printf("\n");
        return NULL_VAL;
    }

    for (int i = 0; i < argCount; i++) {
        Value value = args[i];
        printValue(value);
        printf("\n");
    }

    return NULL_VAL;
}

static Value writeNative(int argCount, Value *args)
{
    if (argCount == 0)
    {
        return NULL_VAL;
    }

    for (int i = 0; i < argCount; i++)
    {
        Value value = args[i];
        printValue(value);
        printf(" ");
    }

    return NULL_VAL;
}

const char *nativeNames[] = {
    "clock",
    "print",
    "write",
};

NativeFn nativeFunctions[] = {
    clockNative,
    printNative,
    writeNative,
};

const char* nativeVoidNames[] = {
    //
};

NativeVoidFn nativeVoidFunctions[] = {
    //
};

void defineAllNatives() {
    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); i++) {
        defineNative(nativeNames[i], nativeFunctions[i]);
    }

    for (uint8_t i = 0; i < sizeof(nativeVoidNames) / sizeof(nativeVoidNames[0]); i++) {
        defineNativeVoid(nativeVoidNames[i], nativeVoidFunctions[i]);
    }
}