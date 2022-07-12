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

/* ConstantDefinition -> "const" Name "=" Constant */
static void parseConstantDefinition() {
    
}

/* Definition -> ConstantDefinition | VariableDefinition | ProcedureDefinition */
static void parseDefinition() {
    if (ch == T_CONST) {
        parseConstantDefinition();
    } else if (ch == T_INTEGER || ch == T_BOOLEAN) {
        parseVariableDefinition();
    } else if (ch == T_PROC) {
        parseProcedureDefinition();
    } else {
        syntaxError = true;
        //TODO: Print
    }
}

/* DefinitionPart -> { Definition ";"} */
static void parseDefinitionPart() {
    while (ch == T_CONST || ch == T_INTEGER || ch == T_BOOLEAN || ch == T_PROC) {
        parseDefinition();
        expect(T_SEMI);
    }
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