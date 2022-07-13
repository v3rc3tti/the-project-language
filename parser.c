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
static SymSet exprFirst;
static SymSet termFirst;

static bool inSet(SymSet set, SymbolType sym) {
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

static void markError(SymSet stop) {
    syntaxError = true;  
        
    while (!inSet(stop, sym)) {
        next();
    }
}

static void skipUntil(SymSet stop) {
    if (!inSet(stop, sym)) {
        markError(stop);
        
        printf("%d: Expected ", getLine());
        for (int i = 0; i < T_COUNT; i++) {
            if (stop.arr[i]) {
                printf("%s ", getSymName(sym));
            }
        }
        printf("but found %s\n", getSymName(sym));
    }
}

static void expect(SymbolType exp, SymSet stop) {
    if (sym == exp) {
        next();
    } else {
        markError(stop);
        printf("%d: Expected %s but found %s\n", getLine(), getSymName(exp), getSymName(sym));
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

static void parseName(SymSet stop) {
    expect(T_NAME, stop);
}

/* BooleanSymbol -> "false" | "true" */
static void parseBooleanSymbol(SymSet stop) {
    if (sym == T_TRUE) {
        expect(T_TRUE, stop);
    } else if (sym == T_FALSE) {
        expect(T_FALSE, stop);
    } else {
        printf("%d: Expected boolean value but found %s\n", getLine(), getSymName(sym));
        markError(stop);
    }
}

/* Constant -> Numeral | BooleanSymbol | Name */
static void parseConstant(SymSet stop) {
    if (sym == T_NUM) {
        expect(T_NUM, stop);
    } else if (check(2, T_TRUE, T_FALSE)) {
        parseBooleanSymbol(stop);
    } else if (sym == T_NAME) {
        parseName(stop);
    } else {
        printf("%d: Expected constant but found %s\n", getLine(), getSymName(sym));
        markError(stop);
    }
}

/* IndexedSelector -> "[" Expression "]" */
static void parseIndexedSelector(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_RSQUAR);
    SymSet stop2 = unionSet(stop, exprFirst);
    
    expect(T_LSQUAR, stop2);
    parseExpression(stop1);
    expect(T_RSQUAR, stop);
}

/* VariableAccess -> Name [ IndexedSelector ] */
static void parseVariableAccess(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_LSQUAR);
    
    parseName(stop1);
    if (sym == T_LSQUAR) {
        parseIndexedSelector(stop);
    }
}

/* Factor -> Numeral | BooleanSymbol | VariableAccess | "(" Expression ")" | "~" Factor */
static void parseFactor(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_RPAREN);
    SymSet stop2 = unionSet(stop1, exprFirst);
    SymSet stop3 = unionSet(stop, termFirst);
    
    if (sym == T_NUM) {
        expect(T_NUM, stop);
    } else if (check(2, T_TRUE, T_FALSE)) {
        parseBooleanSymbol(stop);
    } else if (sym == T_NAME) {
        parseVariableAccess(stop);
    } else if (sym == T_LPAREN) {
        expect(T_LPAREN, stop2);
        parseExpression(stop1);
        expect(T_RPAREN, stop);
    } else if (sym == T_NOT) {
        expect(T_NOT, stop3);
        parseFactor(stop);
    } else {
        printf("%d: Expected number, boolean value, identifier, ( or ~ but found %s\n",
            getLine(), getSymName(sym));
        markError(stop);
    }
}

/* MultiplyingOperator -> "*" | "/" | "\" */
static void parseMultiplyingOperator(SymSet stop) {
    if (sym == T_MULT) {
        expect(T_MULT, stop);
    } else if (sym == T_DIV) {
        expect(T_DIV, stop);
    } else if (sym == T_MOD) {
        expect(T_MOD, stop);
    } else {
        printf("%d: Expected * / or \\ but found %s\n", getLine(), getSymName(sym));
        markError(stop);
    }
}

/* Term -> Factor { MultiplyingOperator Factor } */
static void parseTerm(SymSet stop) {
    SymSet stop1 = newSet(stop, 3, T_MULT, T_DIV, T_MOD);
    SymSet stop2 = unionSet(stop1, termFirst);
    
    parseFactor(stop1);
    while (check(3, T_MULT, T_DIV, T_MOD)) {
        parseMultiplyingOperator(stop2);
        parseFactor(stop1);
    }
}

/* AddingOperator -> "+" | "-" */
static void parseAddingOperator(SymSet stop) {
    if (sym == T_PLUS) {
        expect(T_PLUS, stop);
    } else if (sym == T_MINUS) {
        expect(T_MINUS, stop);
    } else {
        printf("%d: Expected + or - but found %s\n", getLine(), getSymName(sym));
        markError(stop);
    }
}

/* SimpleExpression -> ["-"] Term { AddingOperator Term } */
static void parseSimpleExpression(SymSet stop) {
    SymSet stop1 = newSet(stop, 2, T_PLUS, T_MINUS);
    SymSet stop2 = unionSet(stop1, termFirst);
    
    if (sym == T_MINUS) {
        expect(T_MINUS, stop2);
    }
    parseTerm(stop1);
    while (check(2, T_PLUS, T_MINUS)) {
        parseAddingOperator(stop2);
        parseTerm(stop1);
    }
}

/* RelationalOperator -> "<" | "=" | ">" */
static void parseRelationalOperator(SymSet stop) {
    if (sym == T_LES) {
        expect(T_LES, stop);
    } else if (sym == T_EQ) {
        expect(T_EQ, stop);
    } else if (sym == T_GRE) {
        expect(T_GRE, stop);
    } else {
        printf("%d: Expected < = or > but found %s\n", getLine(), getSymName(sym));
        markError(stop);
    }
}

/* PrimaryExpression -> SimpleExpression [ RelationalOperator SimpleExpression ] */
static void parsePrimaryExpression(SymSet stop) {
    SymSet stop1 = newSet(stop, 3, T_LES, T_EQ, T_GRE);
    SymSet stop2 = unionSet(stop1, exprFirst);
    
    parseSimpleExpression(stop1);
    if (check(3, T_LES, T_EQ, T_GRE)) {
        parseRelationalOperator(stop2);
        parseSimpleExpression(stop1);
    }
}

/* PrimaryOperator -> "&" | "|" */
static void parsePrimaryOperator(SymSet stop) {
    if (sym == T_AND) {
        expect(T_AND, stop);
    } else if (sym == T_OR) {
        expect(T_OR, stop);
    } else {
        printf("%d: Expected & or | but found %s\n", getLine(), getSymName(sym));
        markError(stop);
    }
}

/* Expression -> PrimaryExpression { PrimaryOperator PrimaryExpression } */
static void parseExpression(SymSet stop) {
    SymSet stop1 = newSet(stop, 2, T_AND, T_OR);
    SymSet stop2 = unionSet(stop1, exprFirst);
    
    parsePrimaryExpression(stop1);
    while (check(2, T_AND, T_OR)) {
        parsePrimaryOperator(stop2);
        parsePrimaryExpression(stop1);
    }
}

/* GuardedCommand -> Expression "->" StatementPart */
static void parseGuardedCommand(SymSet stop) {
    SymSet stop1 = unionSet(stop, stmtFirst);
    SymSet stop2 = newSet(stop1, 1, T_ARROW);
    
    parseExpression(stop2);
    expect(T_ARROW, stop1);
    parseStatementPart(stop);
}

/* GuardedCommandList -> GuardedCommand { "[]" GuardedCommand } */
static void parseGuardedCommandList(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_GUARD);
    SymSet stop2 = unionSet(stop1, exprFirst);
    
    parseGuardedCommand(stop1);
    while (sym == T_GUARD) {
        expect(T_GUARD, stop2);
        parseGuardedCommand(stop1);
    }
}

/* DoStatement -> "do" GuardedCommandList "od" */
static void parseDoStatement(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_OD);
    SymSet stop2 = unionSet(stop1, exprFirst);
    
    expect(T_DO, stop2);
    parseGuardedCommandList(stop1);
    expect(T_OD, stop);
}

/* IfStatement -> "if" GuardedCommandList "fi" */
static void parseIfStatement(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_FI);
    SymSet stop2 = unionSet(stop1, exprFirst);
    
    expect(T_IF, stop2);
    parseGuardedCommandList(stop1);
    expect(T_FI, stop);
}

/* ProcedureStatement -> "call" Name */
static void parseProcedureStatement(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_NAME);
    
    expect(T_CALL, stop1);
    parseName(stop);
}

/* AssignmentStatement -> VariableAccessList ":=" ExpressionList */
static void parseAssignmentStatement(SymSet stop) {
    SymSet stop1 = unionSet(stop, exprFirst);
    SymSet stop2 = newSet(stop1, 1, T_ASSIGN);
    
    parseVariableAccessList(stop2);
    expect(T_ASSIGN, stop1);
    parseExpressionList(stop);
}

/* ExpressionList -> Expression { "," Expression } */
static void parseExpressionList(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_COMMA);
    SymSet stop2 = unionSet(stop1, exprFirst);
    
    parseExpression(stop1);
    while (sym == T_COMMA) {
        expect(T_COMMA, stop2);
        parseExpression(stop1);
    }
}

/* WriteStatement -> "write" ExpressionList */
static void parseWriteStatement(SymSet stop) {
    SymSet stop1 = unionSet(stop, exprFirst);
    
    expect(T_WRITE, stop1);
    parseExpressionList(stop);
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
        printf("%d: Expected start of statement but found %s\n", getLine(), getSymName(sym));
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
        printf("%d: Expected Integer or Boolean but found %s\n", getLine(), getSymName(sym));
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
        printf("%d: Expected array or identifier but found %s\n", getLine(), getSymName(sym));
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
        printf("%d: Expected const Integer Boolean or proc but found %s\n", getLine(), getSymName(sym));
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
    
    endSet = newSet((SymSet){0}, 1, T_EOF);
    defFirst = newSet(endSet, 4, T_CONST, T_INTEGER, T_BOOLEAN, T_PROC);
    stmtFirst = newSet(endSet, 7, T_SKIP, T_WRITE, T_NAME, T_CALL, T_IF, T_DO, T_READ);
    constFirst = newSet(endSet, 4, T_NUM, T_NAME, T_FALSE, T_TRUE);
    exprFirst = newSet(endSet, 7, T_MINUS, T_NUM, T_NAME, T_FALSE, T_TRUE, T_LPAREN, T_NOT);
    termFirst = newSet(endSet, 6, T_NUM, T_NAME, T_FALSE, T_TRUE, T_LPAREN, T_NOT);
    
    next();
    parseProgram(endSet);
    //TODO: Check T_EOF
    return !lexError && !syntaxError;
}