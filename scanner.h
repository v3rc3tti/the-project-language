#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

typedef enum {
    T_BEGIN=1, // 'begin'
    T_END,     // 'end'
    T_CONST,   // 'const'
    T_SKIP,    // 'skip'
    T_ARRAY,   // 'array'
    T_PROC,    // 'proc'
    T_READ,    // 'read'
    T_WRITE,   // 'write'
    T_CALL,    // 'call'
    T_IF,      // 'if'
    T_FI,      // 'fi'
    T_DO,      // 'do'
    T_OD,      // 'od'
    T_LSQUAR,  // '['
    T_RSQUAR,  // ']'
    T_EQ,      // '='
    T_LES,     // '<'
    T_GRE,     // '>'
    T_GUARD,   // '[]'
    T_ARROW,   // '->'
    T_ASSIGN,  // ':='
    T_AND,     // '&'
    T_OR,      // '|'
    T_SEMI,    // ';'
    T_MINUS,   // '-'
    T_PLUS,    // '+'
    T_MULT,    // '*'
    T_DIV,     // '/'
    T_MOD,     // '\'
    T_LPAREN,  // '('
    T_RPAREN,  // ')'
    T_NOT,     // '~'
    T_COMMA,   // ','
    T_POINT,   // '.'
    T_FALSE,   // 'false'
    T_TRUE,    // 'true'
    T_INTEGER, // 'Integer'
    T_BOOLEAN, // 'Boolean'
    T_NUM,
    T_NAME,
    T_EOF,     // End of input
    T_COUNT
} SymbolType;

extern bool lexError;

void initScan(char *str);
SymbolType scanNext();
int getLine();
const char *getSymName(SymbolType type);

#endif
