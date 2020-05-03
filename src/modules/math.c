#include <math.h>
#include <stdlib.h>

#include "math.h"
#include "../native.h"
#include "../object.h"
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
        runtimeError("math_abs() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("math_abs() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(abs((int)AS_NUMBER(number)));
}

static Value
mathAcos(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("math_acos() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("math_abs() expects a number argument.");
        return NULL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(acos((int)AS_NUMBER(number)));
}

static Value
mathAsin(int argCount, Value *args)
{
    if (argCount == 0)
    {
        runtimeError("math_asin() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("math_asin() expects a number argument.");
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
        runtimeError("math_atan() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("math_atan() expects a number argument.");
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
        runtimeError("math_ceil() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("math_ceil() expects a number argument.");
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
        runtimeError("math_cos() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("math_cos() expects a number argument.");
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
        runtimeError("math_floor() expects exactly one argument.");
        return NULL_VAL;
    }

    if (!IS_NUMBER(args[0]))
    {
        runtimeError("math_floor() expects a number argument.");
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
        runtimeError("math_max() expects at least one argument.");
        return NULL_VAL;
    }

    double max = AS_NUMBER(args[0]);
    double value = AS_NUMBER(args[0]);

    for (int i = 1; argCount <= i; i++)
    {
        value = AS_NUMBER(args[i]);
        max = (((value) > (max)) ? (value) : (max));
    }

    return NUMBER_VAL(max);
}

static Value
mathPi(int argCount, Value *args)
{
    return NUMBER_VAL(M_PI);
}

const char *mathNames[] = {
    "math_abs",
    "math_acos",
    "math_asin",
    "math_atan",
    "math_ceil",
    "math_cos",
    "math_floor",
    "math_max",
    "math_pi",
};

NativeFn mathFunctions[] = {
    mathAbs,
    mathAcos,
    mathAsin,
    mathAtan,
    mathCeil,
    mathCos,
    mathFloor,
    mathMax,
    mathPi};

void defineAllMathNatives()
{
    for (uint8_t i = 0; i < sizeof(mathNames) / sizeof(mathNames[0]); i++)
    {
        defineNative(mathNames[i], mathFunctions[i]);
    }
}