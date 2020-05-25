#include <ctype.h>
#include <stdlib.h>

#include "../include/ghost.h"
#include "string.h"
#include "../vm.h"

bool static stringLowerCase(GhostVM *vm, int argCount)
{
    ObjString *string = AS_STRING(pop(vm));

    char *temp = malloc(sizeof(char) * (string->length + 1));

    for (int i = 0; string->chars[i]; i++)
    {
        temp[i] = tolower(string->chars[i]);
    }

    temp[string->length] = '\0';

    push(vm, OBJ_VAL(copyString(vm, temp, string->length)));
    return true;
}

bool static stringToNumber(GhostVM *vm, int argCount)
{
    char *numberString = AS_CSTRING(pop(vm));
    char *end;

    double number = strtod(numberString, &end);

    push(vm, NUMBER_VAL(number));
    return true;
}

bool static stringUpperCase(GhostVM *vm, int argCount)
{
    ObjString *string = AS_STRING(pop(vm));

    char *temp = malloc(sizeof(char) * (string->length + 1));

    for (int i = 0; string->chars[i]; i++)
    {
        temp[i] = toupper(string->chars[i]);
    }

    temp[string->length] = '\0';

    push(vm, OBJ_VAL(copyString(vm, temp, string->length)));
    return true;
}

bool static stringLength(GhostVM *vm, int argCount)
{
    ObjString *string = AS_STRING(pop(vm));

    push(vm, NUMBER_VAL(string->length));
    return true;
}

bool declareString(GhostVM *vm, char *method, int argCount)
{
    if (strcmp(method, "upperCase") == 0) {
        return stringUpperCase(vm, argCount);
    } else if (strcmp(method, "toNumber") == 0) {
        return stringToNumber(vm, argCount);
    } else if (strcmp(method, "lowerCase") == 0) {
        return stringLowerCase(vm, argCount);
    } else if (strcmp(method, "length") == 0) {
        return stringLength(vm, argCount);
    }

    runtimeError(vm, "String has no method %s()", method);
    return false;
}