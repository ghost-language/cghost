#ifndef ghost_value_h
#define ghost_value_h

#include "common.h"

// This defines the built-in types and their core representations in memory.
// Since Ghost is dynamically typed, any variable can hold a value of any type,
// and the type can change at runtime. Implementing this efficiently is
// critical for performance.
//
// The main type exposed by this is [Value]. A C variable of that type is a
// storage location that can hold any Ghost value. The stack, global variables,
// and instance fields are all implemented in C as variables of type Value.
//
// The built-in types for booleans, numbers, and nil are unboxed: their value
// is stored directly in the Value, and copying a Value copies the value. Other
// types--classes, instances of classes, functions, lists, and strings--are all
// reference types. They are stored on the heap and the Value just stores a
// pointer to it. Copying the Value copies a reference to the same object. The
// Ghost implementation calls these "Obj", or objects, though to a user, all
// values are objects.

typedef struct sObj Obj;
typedef struct sObjString sObjString;

typedef enum {
    VAL_BOOL,
    VAL_NIL,
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
#define IS_NIL(value)    ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value)    ((value).type == VAL_OBJ)

#define AS_OBJ(value)    ((value).as.obj)
#define AS_BOOL(value)   ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define BOOL_VAL(value)   ((Value) { VAL_BOOL, { .boolean = value }})
#define NIL_VAL           ((Value) { VAL_NIL, { .number = 0 }})
#define NUMBER_VAL(value) ((Value) { VAL_NUMBER, { .number = value }})
#define OBJ_VAL(object)   ((Value) { VAL_OBJ, { .obj = (Obj*)object }})

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