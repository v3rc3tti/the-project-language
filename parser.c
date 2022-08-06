#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include "parser.h"
#include "scanner.h"
#include "scope.h"

static bool syntaxError;
static SymbolType sym;
static int symArg;

typedef struct _AccessList{
    int type;
    struct _AccessList *next;
} AccessList;

static AccessList *newAccessList(int type, AccessList *srcList) {
    AccessList *list = malloc(sizeof(AccessList));
    memset(list, 0, sizeof(AccessList));
    list->type = type;
    list->next = NULL;
    if (!srcList) {
        return list;
    } else {
        AccessList *tmp = srcList;
        while (tmp->next) {
            tmp = tmp->next;
        }
        tmp->next = list;
        return srcList;
    }
}

static void cleanAccessList(AccessList *list) {
    while (list) {
        AccessList *next = list->next;
        free(list);
        list = next;
    }
}

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
        Symbol s = scanNext();
        sym = s.type;
        symArg = s.arg;
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
        printf("%d: Expected ", getLine());
        for (int i = 0; i < T_COUNT; i++) {
            if (stop.arr[i]) {
                printf("%s ", getSymName(i));
            }
        }
        printf("but found %s\n", getSymName(sym));
        
        markError(stop);
    }
}

static void expect(SymbolType exp, SymSet stop) {
    if (sym == exp) {
        next();
        skipUntil(stop);
    } else {
        markError(stop);
        printf("%d: Expected %s but found %s\n", getLine(), getSymName(exp), getSymName(sym));
    }
}

static int expectName(SymSet stop) {
    if (sym == T_NAME) {
        int name = symArg;
        next();
        skipUntil(stop);
        return name;
    } else {
        markError(stop);
        printf("%d: Expected identifier but found %s\n", getLine(), getSymName(sym));
        return NO_NAME;
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

static void parseBlock(SymSet stop);
static void parseExpression(SymSet stop, int *type);
static AccessList *parseExpressionList(SymSet stop);
static AccessList *parseVariableAccessList(SymSet stop);
static void parseStatementPart(SymSet stop);

/* BooleanSymbol -> "false" | "true" */
static int parseBooleanSymbol(SymSet stop) {
    int value = 0;
    if (sym == T_TRUE) {
        value = T_TRUE;
        expect(T_TRUE, stop);
    } else if (sym == T_FALSE) {
        value = T_FALSE;
        expect(T_FALSE, stop);
    } else {
        printf("%d: Expected boolean value but found %s\n", getLine(), getSymName(sym));
        markError(stop);
    }
    return value;
}

/* Constant -> Numeral | BooleanSymbol | Name */
static int parseConstant(SymSet stop, int *type) {
    int value = 0;
    if (sym == T_NUM) {
        *type = T_INTEGER;
        value = symArg;
        expect(T_NUM, stop);
    } else if (check(2, T_TRUE, T_FALSE)) {
        *type = T_BOOLEAN;
        value = parseBooleanSymbol(stop);
    } else if (sym == T_NAME) {
        ObjectRecord *obj = findName(symArg);
        if (obj->kind == OBJ_CONST) {
            *type = obj->as.constant.type;
            value = obj->as.constant.value;
        } else {
            kindError(obj);
            *type = NO_NAME;
        }
        expectName(stop);
    } else {
        printf("%d: Expected constant but found %s\n", getLine(), getSymName(sym));
        markError(stop);
        *type = NO_NAME;
    }
    return value;
}

/* IndexedSelector -> "[" Expression "]" */
static void parseIndexedSelector(SymSet stop, ObjectRecord *obj) {
    SymSet stop1 = newSet(stop, 1, T_RSQUAR);
    SymSet stop2 = unionSet(stop, exprFirst);
    
    expect(T_LSQUAR, stop2);
    int type;
    parseExpression(stop1, &type);
    expect(T_RSQUAR, stop);
}

/* VariableAccess -> Name [ IndexedSelector ] */
static int parseVariableAccess(SymSet stop, int *type) {
    SymSet stop1 = newSet(stop, 1, T_LSQUAR);
    
    ObjectRecord *obj = NULL;
    if (sym == T_NAME) {
        obj = findName(symArg);
    }
    expectName(stop1);
    if (sym == T_LSQUAR) {
        parseIndexedSelector(stop, obj);
    }
    if (obj->kind == OBJ_CONST) {
        *type = obj->as.constant.type;
        return obj->as.constant.value;
    } else if (obj->kind == OBJ_VAR) {
        *type = obj->as.var.type;
        return 0;
    } else {
        kindError(obj);
        *type = NO_NAME;
        return 0;
    }
}

/* Factor -> Numeral | BooleanSymbol | VariableAccess | "(" Expression ")" | "~" Factor */
static int parseFactor(SymSet stop, int *type) {
    SymSet stop1 = newSet(stop, 1, T_RPAREN);
    SymSet stop2 = unionSet(stop1, exprFirst);
    SymSet stop3 = unionSet(stop, termFirst);
    *type = NO_NAME;
    
    if (sym == T_NUM) {
        return parseConstant(stop, type);
    } else if (check(2, T_TRUE, T_FALSE)) {
        *type = T_BOOLEAN;
        return parseBooleanSymbol(stop);
    } else if (sym == T_NAME) {
        parseVariableAccess(stop, type);
    } else if (sym == T_LPAREN) {
        expect(T_LPAREN, stop2);
        parseExpression(stop1, type);
        expect(T_RPAREN, stop);
    } else if (sym == T_NOT) {
        expect(T_NOT, stop3);
        parseFactor(stop, type);
        if (*type != T_BOOLEAN) {
            typeError(*type);
        }
    } else {
        printf("%d: Expected number, boolean value, identifier, ( or ~ but found %s\n",
            getLine(), getSymName(sym));
        markError(stop);
    }
    return 0;
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
static void parseTerm(SymSet stop, int *type) {
    SymSet stop1 = newSet(stop, 3, T_MULT, T_DIV, T_MOD);
    SymSet stop2 = unionSet(stop1, termFirst);
    
    int leftType = NO_NAME;
    int rightType = NO_NAME;
    parseFactor(stop1, &leftType);
    while (check(3, T_MULT, T_DIV, T_MOD)) {
        parseMultiplyingOperator(stop2);
        parseFactor(stop1, &rightType);

        if (leftType != T_INTEGER) {
            typeError(leftType);
            leftType = NO_NAME;
        }
        if (rightType != T_INTEGER) {
            typeError(rightType);
            leftType = NO_NAME;
        }
    }
    *type = leftType;
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
static void parseSimpleExpression(SymSet stop, int *type) {
    SymSet stop1 = newSet(stop, 2, T_PLUS, T_MINUS);
    SymSet stop2 = unionSet(stop1, termFirst);
    
    if (sym == T_MINUS) {
        expect(T_MINUS, stop2);
    }
    
    int leftType = NO_NAME;
    int rightType = NO_NAME;
    parseTerm(stop1, &leftType);
    while (check(2, T_PLUS, T_MINUS)) {
        parseAddingOperator(stop2);
        parseTerm(stop1, &rightType);
        
        if (leftType != T_INTEGER) {
            typeError(leftType);
            leftType = NO_NAME;
        }
        if (rightType != T_INTEGER) {
            typeError(rightType);
            leftType = NO_NAME;
        }
    }
    *type = leftType;
}

/* RelationalOperator -> "<" | "=" | ">" */
static int parseRelationalOperator(SymSet stop) {
    if (sym == T_LES) {
        expect(T_LES, stop);
        return T_LES;
    } else if (sym == T_EQ) {
        expect(T_EQ, stop);
        return T_EQ;
    } else if (sym == T_GRE) {
        expect(T_GRE, stop);
        return T_GRE;
    } else {
        printf("%d: Expected < = or > but found %s\n", getLine(), getSymName(sym));
        markError(stop);
        return NO_NAME;
    }
}

/* PrimaryExpression -> SimpleExpression [ RelationalOperator SimpleExpression ] */
static void parsePrimaryExpression(SymSet stop, int *type) {
    SymSet stop1 = newSet(stop, 3, T_LES, T_EQ, T_GRE);
    SymSet stop2 = unionSet(stop1, exprFirst);
    
    int rightType = NO_NAME;
    int leftType = NO_NAME;
    parseSimpleExpression(stop1, &leftType);
    if (check(3, T_LES, T_EQ, T_GRE)) {
        int oper = parseRelationalOperator(stop2);
        parseSimpleExpression(stop1, &rightType);
        if (oper == T_EQ) {
            if (leftType != rightType) {
                typeError(rightType);
                leftType = NO_NAME;
            } else if (leftType != NO_NAME) {
                leftType = T_BOOLEAN;
            }
        } else {
            if (leftType != T_INTEGER) {
                typeError(leftType);
                leftType = NO_NAME;
            }
            if (rightType != T_INTEGER) {
                typeError(rightType);
                leftType = NO_NAME;
            }
            if (leftType == T_INTEGER && rightType == T_INTEGER) {
                leftType = T_BOOLEAN;
            }
        }
    }
    *type = leftType;
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
static void parseExpression(SymSet stop, int *type) {
    SymSet stop1 = newSet(stop, 2, T_AND, T_OR);
    SymSet stop2 = unionSet(stop1, exprFirst);
    
    int leftType = NO_NAME;
    int rightType = NO_NAME;
    
    parsePrimaryExpression(stop1, &leftType);
    while (check(2, T_AND, T_OR)) {
        parsePrimaryOperator(stop2);
        parsePrimaryExpression(stop1, &rightType);
        
        if (leftType != T_BOOLEAN) {
            typeError(leftType);
            leftType = NO_NAME;
        }
        if (rightType != T_BOOLEAN) {
            typeError(rightType);
            leftType = NO_NAME;
        }
    }
    *type = leftType;
}

/* GuardedCommand -> Expression "->" StatementPart */
static void parseGuardedCommand(SymSet stop) {
    SymSet stop1 = unionSet(stop, stmtFirst);
    SymSet stop2 = newSet(stop1, 1, T_ARROW);
    
    int type;
    parseExpression(stop2, &type);
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
    if (sym == T_NAME) {
        findName(symArg);
    }
    int procName = expectName(stop);
    ObjectRecord *obj = findName(procName);
    if (obj->kind != OBJ_PROC) {
        kindError(obj);
    }
}

/* AssignmentStatement -> VariableAccessList ":=" ExpressionList */
static void parseAssignmentStatement(SymSet stop) {
    SymSet stop1 = unionSet(stop, exprFirst);
    SymSet stop2 = newSet(stop1, 1, T_ASSIGN);
    
    AccessList *list = parseVariableAccessList(stop2);
    expect(T_ASSIGN, stop1);
    AccessList *srcList = parseExpressionList(stop);
    
    AccessList *tmp1 = list;
    AccessList *tmp2 = srcList;
    while (tmp2) {
        if (!tmp1) {
            //TODO: Number doesn't match
            break;
        } else if (tmp1->type != tmp2 -> type) {
            //TODO: Types doesn't match
        }
        
        tmp1 = tmp1->next;
        tmp2 = tmp2->next;
    }
    cleanAccessList(list);
    cleanAccessList(srcList);
}

/* ExpressionList -> Expression { "," Expression } */
static AccessList *parseExpressionList(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_COMMA);
    SymSet stop2 = unionSet(stop1, exprFirst);
    
    int type = NO_NAME;
    parseExpression(stop1, &type);
    AccessList *list  = newAccessList(type, NULL);
    while (sym == T_COMMA) {
        expect(T_COMMA, stop2);
        parseExpression(stop1, &type);
        list = newAccessList(type, list);
    }
    return list;
}

/* WriteStatement -> "write" ExpressionList */
static void parseWriteStatement(SymSet stop) {
    SymSet stop1 = unionSet(stop, exprFirst);
    
    expect(T_WRITE, stop1);
    AccessList *list = parseExpressionList(stop);
    AccessList *tmp = list;
    while (tmp) {
        if (tmp->type != T_INTEGER) {
            typeError(tmp->type);
        }
        tmp = tmp->next;
    }
    cleanAccessList(list);
}

/* VariableAccessList -> VariableAccess { "," VariableAccess } */
static AccessList *parseVariableAccessList(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_COMMA);
    SymSet stop2 = newSet(stop1, 1, T_NAME);
    
    int type = 0;
    parseVariableAccess(stop1, &type);
    AccessList *accList = newAccessList(type, NULL);
    while (sym == T_COMMA) {
        expect(T_COMMA, stop2);
        parseVariableAccess(stop1, &type);
        accList = newAccessList(type, accList);
    }
    return accList;
}

/* ReadStatement -> "read" VariableAccessList */
static void parseReadStatement(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_NAME);
    
    expect(T_READ, stop1);
    AccessList *list = parseVariableAccessList(stop);
    AccessList *tmp = list;
    while (tmp) {
        if (tmp->type != T_INTEGER) {
            typeError(tmp->type);
        }
        tmp = tmp->next;
    }
    cleanAccessList(list);
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
    int name = expectName(stop1);
    defineName(name, OBJ_PROC);
    parseBlock(stop);
}

/* VariableList -> Name { "," Name } */
static void parseVariableList(SymSet stop, int type) {
    SymSet stop1 = newSet(stop, 1, T_COMMA);
    SymSet stop2 = newSet(stop1, 1, T_NAME);
    
    int name = expectName(stop1);
    ObjectRecord *obj = defineName(name, OBJ_VAR);
    obj->as.var.type = type;
    while (sym == T_COMMA) {
        expect(T_COMMA, stop2);
        name = expectName(stop1);
        obj = defineName(name, OBJ_VAR);
        obj->as.var.type = type;
    }
}

/* TypeSymbol -> "Integer" | "Boolean" */
static int parseTypeSymbol(SymSet stop) {
    int type = NO_NAME;
    if (sym == T_INTEGER) {
        type = T_INTEGER;
        expect(T_INTEGER, stop);
    } else if (sym == T_BOOLEAN) {
        type = T_BOOLEAN;
        expect(T_BOOLEAN, stop);
    } else {
        printf("%d: Expected Integer or Boolean but found %s\n", getLine(), getSymName(sym));
        markError(stop);
    }
    return type;
}

/* ArrVarList -> Name ("," ArrVarList | "[" Constant "]") */
static int parseArrVarList(SymSet stop, int type) {
    SymSet stop1 = newSet(stop, 1, T_RSQUAR);
    SymSet stop2 = unionSet(stop1, constFirst);
    SymSet stop3 = newSet(stop, 2, T_COMMA, T_LSQUAR);
    
    int name = expectName(stop3);
    ObjectRecord *obj = defineName(name, OBJ_ARR);
    int constValue = 0;
    if (sym == T_COMMA) {
        expect(T_COMMA, stop);
        constValue = parseArrVarList(stop3, type);
    } else if (sym == T_LSQUAR) {
        expect(T_LSQUAR, stop2);
        int constType;
        constValue = parseConstant(stop1, &constType);
        expect(T_RSQUAR, stop);
    }
    obj->as.arr.type = type;
    obj->as.arr.count = constValue;
    return constValue;
}

/* VariableDefinition -> TypeSymbol ( VariableList | "array" ArrVarList ) */
static void parseVariableDefinition(SymSet stop) {
    SymSet stop1 = newSet(stop, 1, T_NAME);
    SymSet stop2 = newSet(stop1, 1, T_ARRAY);
    
    int type = parseTypeSymbol(stop2);
    if (sym == T_ARRAY) {
        expect(T_ARRAY, stop1);
        parseArrVarList(stop, type);
    } else if (sym == T_NAME) {
        parseVariableList(stop, type);
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
    int name = expectName(stop2);
    expect(T_EQ, stop1);
    int type = 0;
    int value = parseConstant(stop, &type);
    
    ObjectRecord *obj = defineName(name, OBJ_CONST);
    obj->as.constant.value = value;
    obj->as.constant.type = type;
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
    
    startBlock();
    expect(T_BEGIN, stop3);
    parseDefinitionPart(stop2);
    parseStatementPart(stop1);
    expect(T_END, stop);
    finishBlock();
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
    return !lexError && !syntaxError && !analysisError;
}