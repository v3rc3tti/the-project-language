#include <stdio.h>
#include "scanner.h"

#define NAME_LEN 10

typedef struct {
    char str[NAME_LEN+1];
    SymbolType type;
} Name;

bool lexError;

static char *source;
static char ch;
static int lineNumber;
static Name names[17];
static int nameCount;
static const char* symNames[T_COUNT] = {
    "begin", "end", "const", "skip", "array", "proc", "read", "write", "call", "if",
    "fi", "do", "od", "[", "]", "=", "<", ">", "[]", "->", ":=", "&", "|", ";", "-",
    "+", "*", "/", "\\", "(", ")", "~", ",", ".", "false", "true", "Integer",
    "Boolean", "number", "identifier", "end of file"
};

static void reserveName(const char *str, SymbolType type) {
    strncpy(names[nameCount].str, str, NAME_LEN+1);
    names[nameCount].type = type;
    nameCount++;
}

static SymbolType getSymbolType(char *str, int strLen) {
    if (strLen > NAME_LEN) {
        strLen = NAME_LEN;
    }
    str[strLen] = '\0';
    for (int i = 0; i < nameCount; i++) {
        if (!strcmp(names[i].str, str)) {
            return names[i].type;
        }
    }
    return T_NAME;
}

static void advance() {
    if (*source != '\0') {
        source++;
        ch = *source;
    }
    if (ch == '\n') {
        lineNumber++;
    }
}

static bool isEOF() {
    return ch == '\0';
}

static bool isAlpha() {
    return (ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}

static bool isDigit() {
    return (ch >= '0' && ch <= '9');
}

static void skipBlanks() {
    while (!isEOF() && (ch <= 0x20 || ch == '$')) {
        if (ch == '$') {
            while (!isEOF() && ch != '\n') {
                advance();
            }
        }
        advance();
    }
}

void initScan(char *str) {
    lexError = false;
    source = str;
    ch = *source;
    lineNumber = 1;
    nameCount = 0;
    reserveName("begin", T_BEGIN);
    reserveName("end", T_END);
    reserveName("const", T_CONST);
    reserveName("skip", T_SKIP);
    reserveName("array", T_ARRAY);
    reserveName("proc", T_PROC);
    reserveName("read", T_READ);
    reserveName("write", T_WRITE);
    reserveName("call", T_CALL);
    reserveName("if", T_IF);
    reserveName("fi", T_FI);
    reserveName("do", T_DO);
    reserveName("od", T_OD);
    reserveName("false", T_FALSE);
    reserveName("true", T_TRUE);
    reserveName("Integer", T_INTEGER);
    reserveName("Boolean", T_BOOLEAN);
}

SymbolType scanNext() {
    while (true) {
        skipBlanks();
        switch (ch) {
            case '[':
                advance();
                if (ch == ']') {
                    advance();
                    return T_GUARD;
                } else {
                    return T_LSQUAR;
                }
            case ']': advance(); return T_RSQUAR;
            case '=': advance(); return T_EQ;
            case '<': advance(); return T_LES;
            case '>': advance(); return T_GRE;
            case '-':
                advance();
                if (ch == '>') {
                    advance();
                    return T_ARROW;
                } else {
                    return T_MINUS;
                }
            case ':':
                advance();
                if (ch == '=') {
                    advance();
                    return T_ASSIGN;
                } else {
                    lexError = true;
                    printf("Unrecognized symbol '%c'. Did you mean ':='? (%d)\n", ch, lineNumber);
                    break;
                }
            case '&': advance(); return T_AND;
            case '|': advance(); return T_OR;
            case ';': advance(); return T_SEMI;
            case '+': advance(); return T_PLUS;
            case '*': advance(); return T_MULT;
            case '/': advance(); return T_DIV;
            case '\\': advance(); return T_MOD;
            case '(': advance(); return T_LPAREN;
            case ')': advance(); return T_RPAREN;
            case '~': advance(); return T_NOT;
            case ',': advance(); return T_COMMA;
            case '.': advance(); return T_POINT;
            case '\0': advance(); return T_EOF;
            default:
                if (isDigit()) {
                    while (isDigit()) {
                        advance();
                    }
                    return T_NUM;
                } else if (isAlpha()) {
                    char nameStr[NAME_LEN+1];
                    int nameLen = 0;
                    while (isAlpha() || isDigit()) {
                        if (nameLen < NAME_LEN) { 
                            nameStr[nameLen] = ch;
                        }
                        nameLen++;
                        advance();
                    }
                    return getSymbolType(nameStr, nameLen);
                } else {
                    lexError = true;
                    printf("Unrecognized symbol '%d'.(%d)\n", ch, lineNumber);
                    advance();
                    break;
                }
        }
    }
}

int getLine() {
    return lineNumber;
}

const char *getSymName(SymbolType type) {
    return symNames[type-1];
}