#ifndef ghost_table_h
#define ghost_table_h

#include "common.h"
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(GhostVM *vm, Table *table);
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableSet(GhostVM *vm, Table *table, ObjString *key, Value value);
bool tableDelete(Table* table, ObjString* key);
void tableAddAll(GhostVM *vm, Table *from, Table *to);
ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);

void tableRemoveWhite(Table* table);
void markTable(GhostVM *vm, Table *table);
#endif