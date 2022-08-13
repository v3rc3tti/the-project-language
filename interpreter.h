#ifndef INTERPRETER_H
#define INTERPRETER_H

#define MAX_STORE 4000

typedef enum {
    OP_ADD = 1,
    OP_AND,
    OP_ARROW,
    OP_ASSIGN,
    OP_BAR,
    OP_CALL,
    OP_CONSTANT,
    OP_DIVIDE,
    OP_ENDPROC,
    OP_ENDPROG,
    OP_EQUAL,
    OP_FI,
    OP_GREATER,
    OP_INDEX,
    OP_LESS,
    OP_MINUS,
    OP_MODULO,
    OP_MULTIPLY,
    OP_NOT,
    OP_OR,
    OP_PROC,
    OP_PROG,
    OP_READ,
    OP_SUBTRACT,
    OP_VALUE,
    OP_VARIABLE,
    OP_WRITE
} OpCode;

#endif
