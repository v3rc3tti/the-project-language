#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"

#define NAME_LEN 10
#define STORE_LEN 4096

typedef struct Name_ {
    const char* spelling;
    int index;
    bool isReserved;
    struct Name_ *next;
} Name;

typedef struct {
    char *start;
    int capacity;
    int loaded;
} SpellingStore;

Name *nameTable;
SpellingStore spelStore;
bool lexError;

static char *source;
static char ch;
static int lineNumber;
static int nameCount;
static const char* symNames[T_COUNT] = {
    "begin", "end", "const", "skip", "array", "proc", "read", "write", "call", "if",
    "fi", "do", "od", "[", "]", "=", "<", ">", "[]", "->", ":=", "&", "|", ";", "-",
    "+", "*", "/", "\\", "(", ")", "~", ",", ".", "false", "true", "Integer",
    "Boolean", "number", "identifier", "end of file"
};

static void initSpellingStore(int capacity) {
    spelStore.start = malloc(capacity);
    spelStore.capacity = capacity;
    spelStore.loaded = 0;
}

static void cleanSpellingStore() {
    free(spelStore.start);
    spelStore.capacity = 0;
    spelStore.loaded = 0;
}

static char *saveSpelling(const char *str) {
    int strLen = strlen(str) + 1;
    if (spelStore.loaded + strLen >= spelStore.capacity) {
        //TODO: Expand memory
    }
    char *strStart = &spelStore.start[spelStore.loaded];
    strcpy(strStart, str);
    spelStore.loaded += strLen;
    return strStart;
}

static Name *reserveName(const char *str, int index, bool isReserved) {
    Name *newName = malloc(sizeof(Name));
    newName->spelling = saveSpelling(str);
    newName->index = index;
    newName->isReserved = isReserved;
    newName->next = nameTable;
    nameTable = newName;
    return nameTable;
}

static SymbolType getSymbolType(char *str, int strLen) {
    if (strLen > NAME_LEN) {
        strLen = NAME_LEN;
    }
    str[strLen] = '\0';
    
    Name *node = nameTable;
    bool found = false;
    while (node) {
        if (!strcmp(node->spelling, str)) {
            found = true;
            break;
        } else {
            node = node->next;
        }
    }
    
    if (!found) {
        node = reserveName(str, nameCount, false);
        nameCount++;
    }
    if (node->isReserved) {
        return node->index;
    } else {
        return T_NAME;
    }
}

static void cleanNames() {
    Name *node = nameTable;
    while (node) {
        Name *next = node->next;
        free(node);
        node = next;
    }
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
    nameTable = NULL;
    nameCount = 0;
    initSpellingStore(STORE_LEN);
    reserveName("begin", T_BEGIN, true);
    reserveName("end", T_END, true);
    reserveName("const", T_CONST, true);
    reserveName("skip", T_SKIP, true);
    reserveName("array", T_ARRAY, true);
    reserveName("proc", T_PROC, true);
    reserveName("read", T_READ, true);
    reserveName("write", T_WRITE, true);
    reserveName("call", T_CALL, true);
    reserveName("if", T_IF, true);
    reserveName("fi", T_FI, true);
    reserveName("do", T_DO, true);
    reserveName("od", T_OD, true);
    reserveName("false", T_FALSE, true);
    reserveName("true", T_TRUE, true);
    reserveName("Integer", T_INTEGER, true);
    reserveName("Boolean", T_BOOLEAN, true);
}

void cleanScan() {
    cleanNames();
    cleanSpellingStore();
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