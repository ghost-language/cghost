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

static Value typeNative(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "type() takes 1 argument (%d given).", argCount);
        return NULL_VAL;
    }

    if (IS_BOOL(args[0])) {
        return OBJ_VAL(copyString(vm, "bool", 4));
    } else if (IS_NULL(args[0])) {
        return OBJ_VAL(copyString(vm, "null", 4));
    } else if (IS_NUMBER(args[0])) {
        return OBJ_VAL(copyString(vm, "number", 6));
    } else if (IS_OBJ(args[0])) {
        switch (OBJ_TYPE(args[0])) {
            case OBJ_CLASS:
                return OBJ_VAL(copyString(vm, "class", 5));
            case OBJ_CLOSURE:
                return OBJ_VAL(copyString(vm, "closure", 7));
            case OBJ_FUNCTION:
                return OBJ_VAL(copyString(vm, "function", 8));
            case OBJ_STRING:
                return OBJ_VAL(copyString(vm, "string", 6));
            case OBJ_LIST:
                return OBJ_VAL(copyString(vm, "list", 4));
            case OBJ_NATIVE:
                return OBJ_VAL(copyString(vm, "native", 6));
            default:
                break;
        }
    }

    return OBJ_VAL(copyString(vm, "Unknown Type", 12));
}

/**
 * Finds whether a value is a bool.
 */
static Value isBoolNative(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0 || !IS_BOOL(args[0]))
    {
        return FALSE_VAL;
    }

    return TRUE_VAL;
}

/**
 * Finds whether a value is null.
 */
static Value isNullNative(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0 || !IS_NULL(args[0]))
    {
        return FALSE_VAL;
    }

    return TRUE_VAL;
}

/**
 * Finds whether a value is a number.
 */
static Value isNumberNative(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0 || !IS_NUMBER(args[0]))
    {
        return FALSE_VAL;
    }

    return TRUE_VAL;
}

/**
 * Finds whether a value is an object.
 */
static Value isObjectNative(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0 || !IS_OBJ(args[0]))
    {
        return FALSE_VAL;
    }

    return TRUE_VAL;
}

/**
 * Finds whether a value is a string.
 */
static Value isStringNative(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0 || !IS_OBJ(args[0]))
    {
        return FALSE_VAL;
    }

    if (OBJ_TYPE(args[0]) != OBJ_STRING)
    {
        return FALSE_VAL;
    }

    return TRUE_VAL;
}

/**
 * Finds whether a value is a list.
 */
static Value isListNative(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0 || !IS_OBJ(args[0]))
    {
        return FALSE_VAL;
    }

    if (OBJ_TYPE(args[0]) != OBJ_LIST)
    {
        return FALSE_VAL;
    }

    return TRUE_VAL;
}

const char *nativeNames[] = {
    "clock",
    "input",
    "print",
    "write",
    "error",
    "type",
    "isBool",
    "isNull",
    "isNumber",
    "isObject",
    "isString",
    "isList",
};

NativeFn nativeFunctions[] = {
    clockNative,
    inputNative,
    printNative,
    writeNative,
    errorNative,
    typeNative,
    isBoolNative,
    isNullNative,
    isNumberNative,
    isObjectNative,
    isStringNative,
    isListNative,
};

void defineAllNatives(GhostVM *vm) {
    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); i++) {
        defineNative(vm, nativeNames[i], nativeFunctions[i]);
    }
}