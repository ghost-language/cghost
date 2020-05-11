#ifndef ghost_value_h
#define ghost_value_h

// This defines the built-in types and their core representations in memory.
// Since Ghost is dynamically typed, any variable can hold a value of any type,
// and the type can change at runtime. Implementing this efficiently is
// critical for performance.
//
// The main type exposed by this is [Value]. A C variable of that type is a
// storage location that can hold any Ghost value. The stack, global variables,
// and instance fields are all implemented in C as variables of type Value.
//
// The built-in types for booleans, numbers, and null are unboxed: their value
// is stored directly in the Value, and copying a Value copies the value. Other
// types--classes, instances of classes, functions, lists, and strings--are all
// reference types. They are stored on the heap and the Value just stores a
// pointer to it. Copying the Value copies a reference to the same object. The
// Ghost implementation calls these "Obj", or objects, though to a user, all
// values are objects.

#include "common.h"

typedef struct sObj Obj;
typedef struct sObjString ObjString;

#if NAN_BOXING
#define SIGN_BIT    ((uint64_t)0x8000000000000000)
#define QNAN        ((uint64_t)0x7ffc000000000000)

#define TAG_NULL    1 // 01.
#define TAG_FALSE   2 // 10.
#define TAG_TRUE    3 // 11.

typedef uint64_t Value;

#define IS_BOOL(v)      (((v) & FALSE_VAL) == FALSE_VAL)
#define IS_NULL(v)      ((v) == NULL_VAL)
#define IS_NUMBER(v)    (((v) & QNAN) != QNAN)
#define IS_OBJ(v)       (((v) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(v)      ((v) == TRUE_VAL)
#define AS_NUMBER(v)    valueToNum(v)
#define AS_OBJ(v)       ((Obj*)(uintptr_t)((v) & ~(SIGN_BIT | QNAN)))

#define BOOL_VAL(b)     ((b) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL       ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL        ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NULL_VAL        ((Value)(uint64_t)(QNAN | TAG_NULL))
#define NUMBER_VAL(num) numToValue(num)
#define OBJ_VAL(obj) \
    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

typedef union {
    uint64_t bits;
    double num;
    } DoubleUnion;

    static inline double valueToNum(Value value) {
        DoubleUnion data;
        data.bits = value;
        return data.num;
    }

    static inline Value numToValue(double num) {
        DoubleUnion data;
        data.num = num;
        return data.bits;
    }
#else

typedef enum {
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

#define IS_BOOL(value)   ((value).type == VAL_BOOL)
#define IS_NULL(value)    ((value).type == VAL_NULL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value)    ((value).type == VAL_OBJ)

#define AS_OBJ(value)    ((value).as.obj)
#define AS_BOOL(value)   ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define BOOL_VAL(value)   ((Value) { VAL_BOOL, { .boolean = value }})
#define NULL_VAL           ((Value) { VAL_NULL, { .number = 0 }})
#define NUMBER_VAL(value) ((Value) { VAL_NUMBER, { .number = value }})
#define OBJ_VAL(object)   ((Value) { VAL_OBJ, { .obj = (Obj*)object }})

#endif

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);

#endif