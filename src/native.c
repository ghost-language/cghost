#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "include/ghost.h"
#include "memory.h"
#include "native.h"
#include "object.h"
#include "vm.h"

static Value clockNative(GhostVM *vm, int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value inputNative(GhostVM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "input() takes either 0 or 1 argument (%d given)", argCount);
        return NULL_VAL;
    }

    if (argCount != 0) {
        Value prompt = args[0];

        if (!IS_STRING(prompt)) {
            runtimeError(vm, "input() only takes a string argument");
            return NULL_VAL;
        }

        printf("%s ", AS_CSTRING(prompt));
    }

    uint64_t currentSize = 128;
    char *line = malloc(currentSize);

    if (line == NULL) {
        runtimeError(vm, "Memory error on input()");
        return NULL_VAL;
    }

    int c = EOF;
    uint64_t i = 0;

    while ((c = getchar()) != '\n' && c != EOF) {
        line[i++] = (char) c;

        if (i + 1 == currentSize) {
            currentSize = GROW_CAPACITY(currentSize);
            line = realloc(line, currentSize);

            if (line == NULL) {
                printf("Unable to allocate memory.\n");
                exit(71);
            }
        }
    }

    line[i] = '\0';

    Value input = OBJ_VAL(copyString(vm, line, strlen(line)));
    free(line);
    return input;
}

static Value printNative(GhostVM *vm, int argCount, Value *args) {
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

static Value writeNative(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        printf("%s", " ");
        fflush(stdout);
        return NULL_VAL;
    }

    for (int i = 0; i < argCount; i++)
    {
        Value value = args[i];
        printf("%s", AS_CSTRING(value));
        fflush(stdout);
    }

    return NULL_VAL;
}

static Value errorNative(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        return NULL_VAL;
    }

    runtimeError(vm, AS_CSTRING(args[0]));
    exit(70);

    return NULL_VAL;
}

static Value assertNative(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "assert() takes 1 argument (%d given)", argCount);
        exit(70);
    }

    if (isFalsey(args[0]))
    {
        if (argCount == 2) {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[1]));
            runtimeError(vm, message);
        } else {
            runtimeError(vm, "assert() was false.");
        }

        exit(70);
    }

    return NULL_VAL;
}

const char *nativeNames[] = {
    "clock",
    "input",
    "print",
    "write",
    "error",
    "assert",
};

NativeFn nativeFunctions[] = {
    clockNative,
    inputNative,
    printNative,
    writeNative,
    errorNative,
    assertNative,
};

void defineAllNatives(GhostVM *vm) {
    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); i++) {
        defineNative(vm, nativeNames[i], nativeFunctions[i]);
    }
}