#include <stdbool.h>
#include <stdlib.h>
#include "scope.h"
#include "scanner.h"

#define MAX_LEVEL 10

typedef struct ObjectRecord_ {
    int name;
    struct ObjectRecord_ *prev;
} ObjectRecord;

typedef struct {
    ObjectRecord *prev;
} BlockRecord;

static BlockRecord blockTable[MAX_LEVEL];
static int blockLevel;

static bool nameExists(int name, int level) {
    ObjectRecord *obj = blockTable[level].prev;
    while (obj) {
        if (obj->name == name) {
            return true;
        }
        obj = obj->prev;
    }
    return false;
}

void defineName(int name) {
    if (nameExists(name, blockLevel)) {
        printf("%d: Ambiguous definition\n", getLine());
    } else {
        ObjectRecord *rec = malloc(sizeof(ObjectRecord));
        rec->name = name;
        rec->prev = blockTable[blockLevel].prev;
        blockTable[blockLevel].prev = rec;
    }
}

bool findName(int name) {
    int lvl = blockLevel;
    while (lvl >= 0) {
        if (nameExists(name, lvl)) {
            return true;
        } else {
            lvl--;
        }
    }
    printf("%d: Undefined name!\n", getLine());
    defineName(name);
    return false;
}

void startBlock() {
    if (blockLevel+1 >= MAX_LEVEL) {
        printf("%d: Nesting level is too big!\n", getLine());
    } else {
        blockLevel++;
        blockTable[blockLevel].prev = NULL;
    }
}

void finishBlock() {
    blockLevel--;
}
