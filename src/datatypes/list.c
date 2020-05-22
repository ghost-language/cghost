#include <ctype.h>
#include <stdlib.h>

#include "list.h"
#include "../vm.h"

bool static listLength(int argCount)
{
    ObjList *list = AS_LIST(pop());

    push(NUMBER_VAL(list->values.count));
    return true;
}

bool declareList(char *method, int argCount)
{
    if (strcmp(method, "length") == 0) {
        return listLength(argCount);
    }
    // } else if (strcmp(method, "toNumber") == 0) {
    //     return stringToNumber(argCount);
    // }

    runtimeError("List has no method %s()", method);
    return false;
}