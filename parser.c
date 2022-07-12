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

/* Returns true if the input symbol has any of passed types */
static bool check(int count, ...) {
    va_list types;
    va_start(types, count);
    for (int i = 0; i < count; i++) {
        SymbolType type = va_arg(types, SymbolType);
        if (sym == type) {
            return true;
        }
    }
    va_end(types);
    return false;
}

static void parseBlock();

/* Factor -> Constant | VariableAccess | "(" Expression ")" | "~" Factor */
static void parseFactor() {
    
}

/* MultiplyingOperator -> "*" | "/" | "\" */
static void parseMultiplyingOperator() {
    if (sym == T_MULT) {
        expect(T_MULT);
    } else if (sym == T_DIV) {
        expect(T_DIV);
    } else if (sym == T_MOD) {
        expect(T_MOD);
    } else {
        //TODO: Error
    }
}

/* Term -> Factor { MultiplyingOperator Factor } */
static void parseTerm() {
    parseFactor();
    while (check(3, T_MULT, T_DIV, T_MOD)) {
        parseMultiplyingOperator();
        parseFactor();
    }
}

/* AddingOperator -> "+" | "-" */
static void parseAddingOperator() {
    if (sym == T_PLUS) {
        expect(T_PLUS);
    } else if (sym == T_MINUS) {
        expect(T_MINUS);
    } else {
        //TODO: Error
    }
}

/* SimpleExpression -> ["-"] Term { AddingOperator Term } */
static void parseSimpleExpression() {
    if (sym == T_MINUS) {
        expect(T_MINUS);
    }
    parseTerm();
    while (check(2, T_SUM, T_MINUS)) {
        parseAddingOperator();
        parseTerm();
    }
}

/* RelationalOperator -> "<" | "=" | ">" */
static void parseRelationalOperator() {
    if (sym == T_LES) {
        expect(T_LES);
    } else if (sym == T_EQ) {
        expect(T_EQ);
    } else if (sym == T_GRE) {
        expect(T_GRE);
    } else {
        //TODO: Error
    }
}

/* PrimaryExpression -> SimpleExpression [ RelationalOperator SimpleExpression ] */
static void parsePrimaryExpression() {
    parseSimpleExpression();
    if (check(3, T_LES, T_EQ, T_GRE)) {
        parseRelationalOperator();
        parseSimpleExpression();
    }
}

/* PrimaryOperator -> "&" | "|" */
static void parsePrimaryOperator() {
    if (sym == T_AND) {
        expect(T_AND);
    } else if (sym == T_OR) {
        expect(T_OR);
    } else {
        //TODO: Error
    }
}

/* Expression -> PrimaryExpression { PrimaryOperator PrimaryExpression } */
static void parseExpression() {
    parsePrimaryExpression();
    while (check(2, T_AND, T_OR)) {
        parsePrimaryOperator();
        parsePrimaryExpression();
    }
}

/* GuardedCommand -> Expression "->" StatementPart */
static void parseGuardedCommand() {
    parseExpression();
    expect(T_ARROW);
    parseStatementPart();
}

/* GuardedCommandList -> GuardedCommand { "[]" GuardedCommand } */
static void parseGuardedCommandList() {
    parseGuardedCommand();
    while (sym == T_GUARD) {
        expect(T_GUARD);
        parseGuardedCommand();
    }
}

/* DoStatement -> "do" GuardedCommandList "od" */
static void parseDoStatement() {
    expect(T_DO);
    parseGuardedCommandList();
    expect(T_OD);
}

/* IfStatement -> "if" GuardedCommandList "fi" */
static void parseIfStatement() {
    expect(T_IF);
    parseGuardedCommandList();
    expect(T_FI);
}

/* ProcedureStatement -> "call" Name */
static void parseProcedureStatement() {
    expect(T_CALL);
    parseName();
}

/* AssignmentStatement -> VariableAccessList ":=" ExpressionList */
static void parseAssignmentStatement() {
    parseVariableAccessList();
    expect(T_ASSIGN);
    parseExpressionList();
}

/* ExpressionList -> Expression { "," Expression } */
static void parseExpressionList() {
    parseExpression();
    while (sym == T_COMMA) {
        expect(T_COMMA);
        parseExpression();
    }
}

/* WriteStatement -> "write" ExpressionList */
static void parseWriteStatement() {
    expect(T_WRITE);
    parseExpressionList();
}

/* VariableAccessList -> VariableAccess { "," VariableAccess } */
static void parseVariableAccessList() {
    parseVariableAccess();
    while (sym == T_COMMA) {
        expect(T_COMMA);
        parseVariableAccess();
    }
}

/* ReadStatement -> "read" VariableAccessList */
static void parseReadStatement() {
    expect(T_READ);
    parseVariableAccessList();
}

/* EmptyStatement -> "skip" */
static void parseEmptyStatement() {
    expect(T_SKIP);
}

/* Statement -> EmptyStatement | ReadStatement | WriteStatement | AssignmentStatement | ProcedureStatement | IfStatement | DoStatement */
static void parseStatement() {
    if (sym == T_SKIP) {
        parseEmptyStatement();
    } else if (sym == T_READ) {
        parseReadStatement();
    } else if (sym == T_WRITE) {
        parseWriteStatement();
    } else if (sym == T_NAME) {
        parseAssignmentStatement();
    } else if (sym == T_CALL) {
        parseProcedureStatement();
    } else if (sym == T_IF) {
        parseIfStatement();
    } else if (sym == T_DO) {
        parseDoStatement();
    } else {
        //TODO: Error
    }
}

/* StatementPart -> { Statement ";" } */
static void parseStatementPart() {
    while (check(6, T_SKIP, T_WRITE, T_NAME, T_CALL, T_IF, T_DO)) {
        parseStatement();
        expect(T_SEMI);
    }
}

/* ProcedureDefinition -> "proc" Name Block */
static void parseProcedureDefinition() {
    expect(T_PROC);
    parseName();
    parseBlock();
}

/* VariableList -> Name { "," Name } */
static void parseVariableList() {
    parseName();
    while (sym == T_COMMA) {
        expect(T_COMMA);
        parseName();
    }
}

/* TypeSymbol -> "Integer" | "Boolean" */
static void parseTypeSymbol() {
    if (sym == T_INTEGER) {
        expect(T_INTEGER);
    } else if (sym == T_BOOLEAN) {
        expect(T_BOOLEAN);
    } else {
        //TODO: Error
    }
}

/* VariableDefinition -> TypeSymbol ( VariableList | "array" VariableList "[" Constant "]" ) */
static void parseVariableDefinition() {
    parseTypeSymbol();
    if (sym == T_ARRAY) {
        expect(T_ARRAY);
        parseVariableList();
        expect(T_LSQUAR);
        parseConstant();
        expect(T_RSQUAR);
    } else if (sym == T_NAME) {
        parseVariableList();
    } else {
        //TODO: Error
    }
}

/* ConstantDefinition -> "const" Name "=" Constant */
static void parseConstantDefinition() {
    expect(T_CONST);
    parseName();
    expect(T_EQ);
    parseConstant();
}

/* Definition -> ConstantDefinition | VariableDefinition | ProcedureDefinition */
static void parseDefinition() {
    if (sym == T_CONST) {
        parseConstantDefinition();
    } else if (check(2, T_INTEGER, T_BOOLEAN)) {
        parseVariableDefinition();
    } else if (sym == T_PROC) {
        parseProcedureDefinition();
    } else {
        syntaxError = true;
        //TODO: Print
    }
}

/* DefinitionPart -> { Definition ";"} */
static void parseDefinitionPart() {
    while (check(4, T_CONST, T_INTEGER, T_BOOLEAN, T_PROC)) {
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