#include <ctype.h>
#include <stdlib.h>

#include "string.h"
#include "../vm.h"

bool static stringLower(int argCount)
{
    ObjString *string = AS_STRING(pop());

    char *temp = malloc(sizeof(char) * (string->length + 1));

    for (int i = 0; string->chars[i]; i++)
    {
        temp[i] = tolower(string->chars[i]);
    }

    temp[string->length] = '\0';

    push(OBJ_VAL(copyString(temp, string->length)));
    return true;
}

bool static stringToNumber(int argCount)
{
    char *numberString = AS_CSTRING(pop());
    char *end;

    double number = strtod(numberString, &end);

    push(NUMBER_VAL(number));
    return true;
}

bool static stringUpper(int argCount)
{
    ObjString *string = AS_STRING(pop());

    char *temp = malloc(sizeof(char) * (string->length + 1));

    for (int i = 0; string->chars[i]; i++)
    {
        temp[i] = toupper(string->chars[i]);
    }

    temp[string->length] = '\0';

    push(OBJ_VAL(copyString(temp, string->length)));
    return true;
}

bool static stringLength(int argCount)
{
    ObjString *string = AS_STRING(pop());

    push(NUMBER_VAL(string->length));
    return true;
}

bool declareString(char *method, int argCount)
{
    if (strcmp(method, "upper") == 0) {
        return stringUpper(argCount);
    } else if (strcmp(method, "toNumber") == 0) {
        return stringToNumber(argCount);
    } else if (strcmp(method, "lower") == 0) {
        return stringLower(argCount);
    } else if (strcmp(method, "length") == 0) {
        return stringLength(argCount);
    }

    runtimeError("String has no method %s()", method);
    return false;
}