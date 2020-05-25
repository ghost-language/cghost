#include <ctype.h>
#include <stdlib.h>

#include "../include/ghost.h"
#include "list.h"
#include "../vm.h"

bool static listLength(GhostVM *vm, int argCount)
{
    ObjList *list = AS_LIST(pop(vm));

    push(vm, NUMBER_VAL(list->values.count));
    return true;
}

bool declareList(GhostVM *vm, char *method, int argCount)
{
    if (strcmp(method, "length") == 0) {
        return listLength(vm, argCount);
    }
    // } else if (strcmp(method, "toNumber") == 0) {
    //     return stringToNumber(argCount);
    // }

    runtimeError(vm, "List has no method %s()", method);
    return false;
}