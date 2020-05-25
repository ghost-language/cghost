#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../include/ghost.h"
#include "math.h"
#include "../vm.h"

#undef PI
#define PI 3.14159265

#define AS_DEGREE(value) ((value)*180.0 / PI)
#define AS_RAD(value)    ((value)*PI / 180.0)

static Value
mathAbs(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Math.abs() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError(vm, "Math.abs() expects a number argument.");
        return NULL_VAL;
    }

    double value = AS_NUMBER(args[0]);

    if (value < 0)
    {
        return NUMBER_VAL(value * -1);
    }

    return NUMBER_VAL(value);
}

static Value
mathAcos(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Math.acos() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError(vm, "Math.abs() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(acos((double)AS_NUMBER(number)));
}

static Value
mathAsin(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Math.asin() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError(vm, "Math.asin() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(AS_DEGREE(asin((double)AS_NUMBER(number))));
}

static Value
mathAtan(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Math.atan() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError(vm, "Math.atan() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(AS_DEGREE(atan((double)AS_NUMBER(number))));
}

static Value
mathCeil(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Math.ceil() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError(vm, "Math.ceil() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(ceil((double)AS_NUMBER(number)));
}

static Value
mathCos(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Math.cos() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError(vm, "Math.cos() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(cos(AS_RAD((double)AS_NUMBER(number))));
}

static Value
mathFloor(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Math.floor() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError(vm, "Math.floor() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(floor((double)AS_NUMBER(number)));
}

static Value
mathMax(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Math.max() expects at least one argument.");
        return NULL_VAL;
    }

    double maximum = AS_NUMBER(args[0]);

    for (int i = 1; i < argCount; ++i)
    {
        Value value = args[i];

        if (!IS_NUMBER(value))
        {
            runtimeError(vm, "A non-number value passed to Math.max()");
            return NULL_VAL;
        }

        double current = AS_NUMBER(value);

        if (maximum < current)
        {
            maximum = current;
        }
    }

    return NUMBER_VAL(maximum);
}

static Value
mathMin(GhostVM *vm, int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError(vm, "Math.min() expects at least one argument.");
        return NULL_VAL;
    }

    double minimum = AS_NUMBER(args[0]);

    for (int i = 1; i < argCount; ++i)
    {
        Value value = args[i];

        if (!IS_NUMBER(value))
        {
            runtimeError(vm, "A non-number value passed to Math.min()");
            return NULL_VAL;
        }

        double current = AS_NUMBER(value);

        if (minimum > current)
        {
            minimum = current;
        }
    }

    return NUMBER_VAL(minimum);
}

static Value mathPi(GhostVM *vm, int argCount, Value *args)
{
    return NUMBER_VAL(PI);
}

void registerMathModule(GhostVM *vm)
{
    ObjString *name = copyString(vm, "Math", 4);
    push(vm, OBJ_VAL(name));
    ObjNativeClass *klass = newNativeClass(vm, name);
    push(vm, OBJ_VAL(klass));

    defineNativeMethod(vm, klass, "abs", mathAbs);
    defineNativeMethod(vm, klass, "acos", mathAcos);
    defineNativeMethod(vm, klass, "asin", mathAsin);
    defineNativeMethod(vm, klass, "atan", mathAtan);
    defineNativeMethod(vm, klass, "ceil", mathCeil);
    defineNativeMethod(vm, klass, "cos", mathCos);
    defineNativeMethod(vm, klass, "floor", mathFloor);
    defineNativeMethod(vm, klass, "max", mathMax);
    defineNativeMethod(vm, klass, "min", mathMin);
    defineNativeMethod(vm, klass, "pi", mathPi);

    tableSet(vm, &vm->globals, name, OBJ_VAL(klass));
    pop(vm);
    pop(vm);
}