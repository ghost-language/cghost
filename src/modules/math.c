#include <math.h>
#include <stdlib.h>

#include "math.h"
#include "../native.h"
#include "../object.h"
#include "../vm.h"

static Value mathAbsNative(int argCount, Value *args) {
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

static Value mathAcosNative(int argCount, Value *args) {
    if (argCount == 0) {
        runtimeError("math_acos() expects exactly one argument.");
        return NIL_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("math_abs() expects a number argument.");
        return NIL_VAL;
    }

    Value number = args[0];

    return NUMBER_VAL(acos((int)AS_NUMBER(number)));
}

static Value mathPiNative(int argCount, Value *args) {
    return NUMBER_VAL(M_PI);
}

const char *nativeMathNames[] = {
    "math_abs",
    "math_acos",
    "math_pi",
};

NativeFn nativeMathFunctions[] = {
    mathAbsNative,
    mathAcosNative,
    mathPiNative
};

void defineAllMathNatives() {
    for (uint8_t i = 0; i < sizeof(nativeMathNames) / sizeof(nativeMathNames[0]); i++) {
        defineNative(nativeMathNames[i], nativeMathFunctions[i]);
    }
}