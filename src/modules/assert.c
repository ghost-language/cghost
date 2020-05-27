#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/ghost.h"
#include "assert.h"
#include "../vm.h"

static Value
assertIsTrue(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Assert.isTrue() expects at least one argument (%d given).", argCount);
        return NULL_VAL;
    }

    if (isFalsey(args[0]))
    {
        if (argCount == 2)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[1]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.isTrue() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

static Value
assertIsFalse(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Assert.isFalse() expects at least one argument (%d given).", argCount);
        return NULL_VAL;
    }

    if (!isFalsey(args[0]))
    {
        if (argCount == 2)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[1]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.isFalse() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

static Value
assertEquals(GhostVM *vm, int argCount, Value *args)
{
    if (argCount < 2)
    {
        runtimeError(vm, "Assert.equals() expects at least two arguments (%d given).", argCount);
        return NULL_VAL;
    }

    Value a = args[0];
    Value b = args[1];
    Value result = BOOL_VAL(valuesEqual(a, b));

    if (isFalsey(result))
    {
        if (argCount == 3)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[2]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.equals() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

static Value
assertIsBool(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Assert.isBool() expects at least one argument (%d given).", argCount);
        return NULL_VAL;
    }

    if (!IS_BOOL(args[0]))
    {
        if (argCount == 2)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[2]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.isBool() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

static Value
assertIsNull(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Assert.isNull() expects at least one argument (%d given).", argCount);
        return NULL_VAL;
    }

    if (!IS_NULL(args[0]))
    {
        if (argCount == 2)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[2]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.isNull() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

static Value
assertIsNumber(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Assert.isNumber() expects at least one argument (%d given).", argCount);
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        if (argCount == 2)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[2]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.isNumber() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

static Value
assertIsString(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Assert.isString() expects at least one argument (%d given).", argCount);
        return NULL_VAL;
    }

    if (!IS_OBJ(args[0]) || OBJ_TYPE(args[0]) != OBJ_STRING)
    {
        if (argCount == 2)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[2]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.isString() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

static Value
assertGreaterThan(GhostVM *vm, int argCount, Value *args)
{
    if (argCount < 2)
    {
        runtimeError(vm, "Assert.greaterThan() expects at least two arguments (%d given).", argCount);
        return NULL_VAL;
    }

    Value a = args[0];
    Value b = args[1];
    Value result = BOOL_VAL(a > b);

    if (isFalsey(result))
    {
        if (argCount == 3)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[2]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.greaterThan() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

static Value
assertGreaterThanOrEqual(GhostVM *vm, int argCount, Value *args)
{
    if (argCount < 2)
    {
        runtimeError(vm, "Assert.greaterThanOrEqual() expects at least two arguments (%d given).", argCount);
        return NULL_VAL;
    }

    Value a = args[0];
    Value b = args[1];
    Value result = BOOL_VAL(a >= b);

    if (isFalsey(result))
    {
        if (argCount == 3)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[2]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.greaterThanOrEqual() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

static Value
assertLessThan(GhostVM *vm, int argCount, Value *args)
{
    if (argCount < 2)
    {
        runtimeError(vm, "Assert.lessThan() expects at least two arguments (%d given).", argCount);
        return NULL_VAL;
    }

    Value a = args[0];
    Value b = args[1];
    Value result = BOOL_VAL(a < b);

    if (isFalsey(result))
    {
        if (argCount == 3)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[2]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.lessThan() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

static Value
assertLessThanOrEqual(GhostVM *vm, int argCount, Value *args)
{
    if (argCount < 2)
    {
        runtimeError(vm, "Assert.lessThanOrEqual() expects at least two arguments (%d given).", argCount);
        return NULL_VAL;
    }

    Value a = args[0];
    Value b = args[1];
    Value result = BOOL_VAL(a <= b);

    if (isFalsey(result))
    {
        if (argCount == 3)
        {
            char message[1024];
            sprintf(message, "Failed asserting that %s", AS_CSTRING(args[2]));
            runtimeError(vm, message);
        }
        else
        {
            runtimeError(vm, "Assert.lessThanOrEqual() failed.");
        }

        exit(70);
    }

    return NULL_VAL;
}

void registerAssertModule(GhostVM *vm)
{
    ObjString *name = copyString(vm, "Assert", 6);
    push(vm, OBJ_VAL(name));
    ObjNativeClass *klass = newNativeClass(vm, name);
    push(vm, OBJ_VAL(klass));

    defineNativeMethod(vm, klass, "isTrue", assertIsTrue);
    defineNativeMethod(vm, klass, "isFalse", assertIsFalse);
    defineNativeMethod(vm, klass, "equals", assertEquals);
    defineNativeMethod(vm, klass, "isBool", assertIsBool);
    defineNativeMethod(vm, klass, "isNull", assertIsNull);
    defineNativeMethod(vm, klass, "isNumber", assertIsNumber);
    defineNativeMethod(vm, klass, "isString", assertIsString);
    defineNativeMethod(vm, klass, "greaterThan", assertGreaterThan);
    defineNativeMethod(vm, klass, "greaterThanOrEqual", assertGreaterThanOrEqual);
    defineNativeMethod(vm, klass, "lessThan", assertLessThan);
    defineNativeMethod(vm, klass, "lessThanOrEqual", assertLessThanOrEqual);

    tableSet(vm, &vm->globals, name, OBJ_VAL(klass));
    pop(vm);
    pop(vm);
}