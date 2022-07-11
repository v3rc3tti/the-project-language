#include <stdbool.h>
#include "parser.h"
#include "scanner.h"

static bool syntaxError;
static SymbolType sym;

static void next() {
    if (sym != T_EOF) {
        sym = scanNext();
    }
}

static void expect(SymbolType exp) {
    if (sym == exp) {
        next();
    } else {
        syntaxError = true;
        printf("Expected %d but found %d\n", exp, sym);
    }
}


static void parseDefinitionPart() {
}

/* Block -> "begin" DefinitionPart StatementPart "end" */
static void parseBlock() {
    expect(T_BEGIN);
    parseDefinitionPart();
    parseStatmentPart();
    expect(T_END);
}

/* Program -> Block "." */
static void parseProgram() {
    parseBlock();
    expect(T_POINT);
}

bool parse() {
    syntaxError = false;
    nextSym();
    parseProgram();
    expect(T_EOF);
    return !lexError && !syntaxError;
}