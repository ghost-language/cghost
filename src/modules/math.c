#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "math.h"
#include "../vm.h"

#undef PI
#define PI 3.14159265

#define AS_DEGREE(value) ((value)*180.0 / PI)
#define AS_RAD(value)    ((value)*PI / 180.0)

static Value
mathAbs(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("Math.abs() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("Math.abs() expects a number argument.");
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
mathAcos(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("Math.acos() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("Math.abs() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(acos((double)AS_NUMBER(number)));
}

static Value
mathAsin(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("Math.asin() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("Math.asin() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(AS_DEGREE(asin((double)AS_NUMBER(number))));
}

static Value
mathAtan(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("Math.atan() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("Math.atan() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(AS_DEGREE(atan((double)AS_NUMBER(number))));
}

static Value
mathCeil(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("Math.ceil() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("Math.ceil() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(ceil((double)AS_NUMBER(number)));
}

static Value
mathCos(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("Math.cos() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("Math.cos() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(cos(AS_RAD((double)AS_NUMBER(number))));
}

static Value
mathFloor(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("Math.floor() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("Math.floor() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(floor((double)AS_NUMBER(number)));
}

static Value
mathMax(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("Math.max() expects at least one argument.");
        return NULL_VAL;
    }

    double maximum = AS_NUMBER(args[0]);

    for (int i = 1; i < argCount; ++i)
    {
        Value value = args[i];

        if (!IS_NUMBER(value))
        {
            runtimeError("A non-number value passed to Math.max()");
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
mathMin(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("Math.min() expects at least one argument.");
        return NULL_VAL;
    }

    double minimum = AS_NUMBER(args[0]);

    for (int i = 1; i < argCount; ++i)
    {
        Value value = args[i];

        if (!IS_NUMBER(value))
        {
            runtimeError("A non-number value passed to Math.min()");
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

static Value mathPi(int argCount, Value *args)
{
    return NUMBER_VAL(PI);
}

void registerMathModule()
{
    ObjString *name = copyString("Math", 4);
    push(OBJ_VAL(name));
    ObjNativeClass *klass = newNativeClass(name);
    push(OBJ_VAL(klass));

    defineNativeMethod(klass, "abs", mathAbs);
    defineNativeMethod(klass, "acos", mathAcos);
    defineNativeMethod(klass, "asin", mathAsin);
    defineNativeMethod(klass, "atan", mathAtan);
    defineNativeMethod(klass, "ceil", mathCeil);
    defineNativeMethod(klass, "cos", mathCos);
    defineNativeMethod(klass, "floor", mathFloor);
    defineNativeMethod(klass, "max", mathMax);
    defineNativeMethod(klass, "min", mathMin);
    defineNativeMethod(klass, "pi", mathPi);

    tableSet(&vm.globals, name, OBJ_VAL(klass));
    pop();
    pop();
}