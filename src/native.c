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
        printf(" ");
        fflush(stdout);
        return NULL_VAL;
    }

    for (int i = 0; i < argCount; i++)
    {
        Value value = args[i];

        printValue(value);
        fflush(stdout);
    }

    return NULL_VAL;
}

static Value errorNative(int argCount, Value *args)
{
    if (argCount == 0)
    {
        return NULL_VAL;
    }

    runtimeError(AS_CSTRING(args[0]));
    exit(70);

    return NULL_VAL;
}

static Value assertNative(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("assert() takes 1 argument (%d given)", argCount);
        exit(70);
    }

    if (isFalsey(args[0]))
    {
        if (argCount == 2) {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[1]));
            runtimeError(message);
        } else {
            runtimeError("assert() was false.");
        }

        exit(70);
    }

    return NULL_VAL;
}

const char *nativeNames[] = {
    "clock",
    "print",
    "write",
    "error",
    "assert",
};

NativeFn nativeFunctions[] = {
    clockNative,
    printNative,
    writeNative,
    errorNative,
    assertNative,
};

void defineAllNatives() {
    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); i++) {
        defineNative(nativeNames[i], nativeFunctions[i]);
    }
}