#include <stdbool.h>
#include <stdarg.h>
#include "parser.h"
#include "scanner.h"

static bool syntaxError;
static SymbolType sym;

typedef struct {
    bool arr[T_COUNT];
} SymSet;

static SymSet endSet;
static SymSet defFirst;
static SymSet stmtFirst;
static SymSet constFirst;

static bool inSet(SymSet set, Symbol sym) {
    return set.arr[sym];
}

static SymSet newSet(SymSet parent, int count, ...) {
    SymSet set = parent;
    if (count > 0) {
        va_list args;
        va_start(args, count);
        while (count--) {
            SymbolType type = va_arg(args, SymbolType);
            set.arr[type] = true;
        }
        va_end(args);
    }
    return set;
}

static SymSet unionSet(SymSet a, SymSet b) {
    SymSet set = a;
    for (int i = 0; i < T_COUNT; i++) {
        set.arr[i] |= b.arr[i];
    }
    return set;
}

static void next() {
    if (sym != T_EOF) {
        sym = scanNext();
    }
}

static void skipUntil(SymSet stop) {
    //TODO: Print expected
    if (!inSet(stop, sym)) {
        markError(stop);
    }
}

static void markError(SymSet stop) {
    syntaxError = true;
    while (!inSet(stop, sym)) {
        next();
    }
}

static void expect(SymbolType exp, SymSet stop) {
    if (sym == exp) {
        next();
    } else {
        markError(stop);
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
static void parseExpression();
static void parseExpressionList();
static void parseVariableAccessList();
static void parseStatementPart();

static void parseName() {
    expect(T_NAME);
}

/* BooleanSymbol -> "false" | "true" */
static void parseBooleanSymbol() {
    if (sym == T_TRUE) {
        expect(T_TRUE);
    } else if (sym == T_FALSE) {
        expect(T_FALSE);
    } else {
        markError();
    }
}

/* Constant -> Numeral | BooleanSymbol | Name */
static void parseConstant() {
    if (sym == T_NUM) {
        expect(T_NUM);
    } else if (check(2, T_TRUE, T_FALSE)) {
        parseBooleanSymbol();
    } else if (sym == T_NAME) {
        parseName();
    } else {
        markError();
    }
}

/* IndexedSelector -> "[" Expression "]" */
static void parseIndexedSelector() {
    expect(T_LSQUAR);
    parseExpression();
    expect(T_RSQUAR);
}

/* VariableAccess -> Name [ IndexedSelector ] */
static void parseVariableAccess() {
    parseName();
    if (sym == T_LSQUAR) {
        parseIndexedSelector();
    }
}

/* Factor -> Numeral | BooleanSymbol | VariableAccess | "(" Expression ")" | "~" Factor */
static void parseFactor() {
    if (sym == T_NUM) {
        expect(T_NUM);
    } else if (check(2, T_TRUE, T_FALSE)) {
        parseBooleanSymbol();
    } else if (sym == T_NAME) {
        parseVariableAccess();
    } else if (sym == T_LPAREN) {
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
    } else if (sym == T_NOT) {
        expect(T_NOT);
        parseFactor();
    } else {
        markError();
    }
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
        markError();
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
        markError();
    }
}

/* SimpleExpression -> ["-"] Term { AddingOperator Term } */
static void parseSimpleExpression() {
    if (sym == T_MINUS) {
        expect(T_MINUS);
    }
    parseTerm();
    while (check(2, T_PLUS, T_MINUS)) {
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
        markError();
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
        markError();
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
static void parseVariableAccessList(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_COMMA);
    SymSet stop2 = newSet(stop1, 1, T_NAME);
    
    parseVariableAccess(stop1);
    while (sym == T_COMMA) {
        expect(T_COMMA, stop2);
        parseVariableAccess(stop1);
    }
}

/* ReadStatement -> "read" VariableAccessList */
static void parseReadStatement(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_NAME);
    
    expect(T_READ, stop1);
    parseVariableAccessList(stop);
}

/* EmptyStatement -> "skip" */
static void parseEmptyStatement(SymSet stop) {
    expect(T_SKIP, stop);
}

/* Statement -> EmptyStatement | ReadStatement | WriteStatement | AssignmentStatement | ProcedureStatement | IfStatement | DoStatement */
static void parseStatement(SymSet stop) {
    if (sym == T_SKIP) {
        parseEmptyStatement(stop);
    } else if (sym == T_READ) {
        parseReadStatement(stop);
    } else if (sym == T_WRITE) {
        parseWriteStatement(stop);
    } else if (sym == T_NAME) {
        parseAssignmentStatement(stop);
    } else if (sym == T_CALL) {
        parseProcedureStatement(stop);
    } else if (sym == T_IF) {
        parseIfStatement(stop);
    } else if (sym == T_DO) {
        parseDoStatement(stop);
    } else {
        markError(stop);
    }
}

/* StatementPart -> { Statement ";" } */
static void parseStatementPart(SymSet stop) {
    SymSet stop1 = unionSet(stop, stmtFirst);
    SymSet stop2 = newSet(stop1, 1, T_SEMI);
    
    skipUntil(stop1);
    while (inSet(stmtFirst, sym)) {
        parseStatement(stop2);
        expect(T_SEMI, stop1);
    }
}

/* ProcedureDefinition -> "proc" Name Block */
static void parseProcedureDefinition(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_BEGIN);
    SymSet stop2 = newSet(stop1, 1, T_NAME);
    
    expect(T_PROC, stop2);
    parseName(stop1);
    parseBlock(stop);
}

/* VariableList -> Name { "," Name } */
static void parseVariableList(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_COMMA);
    SymSet stop2 = newSet(stop1, 1, T_NAME);
    
    parseName(stop1);
    while (sym == T_COMMA) {
        expect(T_COMMA, stop2);
        parseName(stop1);
    }
}

/* TypeSymbol -> "Integer" | "Boolean" */
static void parseTypeSymbol(SymSet stop) {
    if (sym == T_INTEGER) {
        expect(T_INTEGER, stop);
    } else if (sym == T_BOOLEAN) {
        expect(T_BOOLEAN, stop);
    } else {
        markError(stop);
    }
}

/* VariableDefinition -> TypeSymbol ( VariableList | "array" VariableList "[" Constant "]" ) */
static void parseVariableDefinition(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_RSQUAR);
    SymSet stop2 = unionSet(stop1, constFirst);
    SymSet stop3 = newSet(stop2, 1, T_LSQUAR);
    SymSet stop4 = newSet(stop, 2, T_ARRAY, T_NAME);
    
    parseTypeSymbol(stop4);
    if (sym == T_ARRAY) {
        expect(T_ARRAY, stop3);
        parseVariableList(stop3);
        expect(T_LSQUAR, stop2);
        parseConstant(stop1);
        expect(T_RSQUAR, stop);
    } else if (sym == T_NAME) {
        parseVariableList(stop);
    } else {
        markError(stop);
    }
}

/* ConstantDefinition -> "const" Name "=" Constant */
static void parseConstantDefinition(SymSet stop) {
    SymSet stop1 = unionSet(stop, constFirst);
    SymSet stop2 = newSet(stop1, 1, T_EQ);
    
    expect(T_CONST, stop2);
    parseName(stop2);
    expect(T_EQ, stop1);
    parseConstant(stop);
}

/* Definition -> ConstantDefinition | VariableDefinition | ProcedureDefinition */
static void parseDefinition(SymSet stop) {
    if (sym == T_CONST) {
        parseConstantDefinition(stop);
    } else if (check(2, T_INTEGER, T_BOOLEAN)) {
        parseVariableDefinition(stop);
    } else if (sym == T_PROC) {
        parseProcedureDefinition(stop);
    } else {
        markError(stop);
    }
}

/* DefinitionPart -> { Definition ";"} */
static void parseDefinitionPart(SymSet stop) {
    SymSet stop1 = unionSet(defFirst, stop);
    SymSet stop2 = newSet(stop1, 1, T_SEMI);
    
    skipUntil(stop1);
    while (inSet(defFirst, sym)) {
        parseDefinition(stop2);
        expect(T_SEMI, stop1);
    }
}

/* Block -> "begin" DefinitionPart StatementPart "end" */
static void parseBlock(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_END);
    SymSet stop2 = unionSet(stop1, stmtFirst);
    SymSet stop3 = unionSet(stop2, defFirst);
    
    expect(T_BEGIN, stop3);
    parseDefinitionPart(stop2);
    parseStatementPart(stop1);
    expect(T_END, stop);
}

/* Program -> Block "." */
static void parseProgram(SymSet stop) {
    parseBlock(newSet(stop, 1, T_POINT));
    expect(T_POINT, stop);
}

bool parse() {
    syntaxError = false;
    
    endSet = newSet({0}, 1, T_EOF);
    defFirst = newSet(endSet, 4, T_CONST, T_INTEGER, T_BOOLEAN, T_PROC);
    stmtFirst = newSet(endSet, 7, T_SKIP, T_WRITE, T_NAME, T_CALL, T_IF, T_DO, T_READ);
    constFirst = newSet(endSet, 4, T_NUM, T_NAME, T_FALSE, T_TRUE);
    
    next();
    parseProgram(defSet);
    expect(T_EOF);
    return !lexError && !syntaxError;
}