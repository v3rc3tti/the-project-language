#ifndef SCOPE_H
#define SCOPE_H

#define NO_NAME -1

typedef struct ObjectRecord_ {
    int name;
    struct ObjectRecord_ *prev;
    enum ObjectKind {
        OBJ_UNDEFINED,
        OBJ_CONST,
        OBJ_PROC,
        OBJ_VAR,
        OBJ_ARR
    } kind;
    union {
        struct {int type; int value;} constant;
        struct {int type;} var;
        struct {int count; int type;} arr;
    } as;
} ObjectRecord;

extern bool analysisError;

ObjectRecord *defineName(int name, int kind);
ObjectRecord *findName(int name);
void startBlock();
void finishBlock();
void typeError(ObjectRecord *obj);

#endif
