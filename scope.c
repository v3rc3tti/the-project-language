#include <stdbool.h>
#include <stdlib.h>
#include "scope.h"
#include "scanner.h"

#define MAX_LEVEL 10

typedef struct {
    ObjectRecord *prev;
} BlockRecord;

bool analysisError;

static BlockRecord blockTable[MAX_LEVEL];
static int blockLevel;

static ObjectRecord *nameExists(int name, int level) {
    ObjectRecord *obj = blockTable[level].prev;
    while (obj) {
        if (obj->name == name) {
            break;
        }
        obj = obj->prev;
    }
    return obj;
}

ObjectRecord *defineName(int name, int kind) {
    if (name != NO_NAME && nameExists(name, blockLevel)) {
        printf("%d: Ambiguous definition '%s'!\n", getLine(), getNameSpel(name));
        analysisError = true;
    }
    ObjectRecord *rec = malloc(sizeof(ObjectRecord));
    rec->name = name;
    rec->kind = kind;
    rec->prev = blockTable[blockLevel].prev;
    blockTable[blockLevel].prev = rec;
    return rec;
}

ObjectRecord *findName(int name) {
    int lvl = blockLevel;
    while (lvl >= 0) {
        ObjectRecord *obj = nameExists(name, lvl);
        if (obj) {
            return obj;
        } else {
            lvl--;
        }
    }
    printf("%d: Undefined name '%s'!\n", getLine(), getNameSpel(name));
    analysisError = true;
    return defineName(name, OBJ_UNDEFINED);
}

void startBlock() {
    if (blockLevel+1 >= MAX_LEVEL) {
        printf("%d: Nesting level is too big!\n", getLine());
        analysisError = true;
    } else {
        blockLevel++;
        blockTable[blockLevel].prev = NULL;
    }
}

void finishBlock() {
    blockLevel--;
}

void kindError(ObjectRecord *obj) {
    if (obj->kind != OBJ_UNDEFINED) {
        printf("%d: Incorrect kind!\n", getLine());
        analysisError = true;
    }
}

void typeError(int type) {
    if (type != NO_NAME) {
        printf("%d: Incorrect type!\n", getLine());
        analysisError = true;
        type = NO_NAME;
    }
}
