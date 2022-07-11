# The Project Language

Implementation of the Project Language compiler defined in the book "Brinch Hansen on Pascal Compilers" using C programming language.

## Lexical Analysis

## Syntax Analysis

The Project Language grammar:
```
Program -> Block "."
Block -> "begin" DefinitionPart StatementPart "end"
DefinitionPart -> { Definition ";"}
Definition -> ConstantDefiniton | VariableDefinition | ProcedureDefiniton
ConstantDefinition -> "const" ConstantName "=" Constant
VariableDefinition -> TypeSymbol VaribleList | TypeSymbol "array" VariableList "[" Constant "]"
TypeSymbol -> "Integer" | "Boolean"
VariableList -> VariableName { "," VariableName }
ProcedureDefinition -> "proc" ProcedureName Block
StatementPart -> { Statement ";" }
Statement -> EmptyStatement | ReadStatement | WriteStatement | AssignmentStatement | ProcedureStatement | IfStatement | DoStatement
EmptyStatement -> "skip"
ReadStatement -> "read" VariableAccessList
VariableAccessList -> VariableAccess { "," VariableAccess }
WriteStatement -> "write" ExpressionList
ExpressionList -> Expression { "," Expression }
AssignmentStatement -> VariableAccessList ":=" ExpressionList
ProcedureStatement -> "call" ProcedureName
IfStatement -> "if" GuardedCommandList "fi"
DoStatement -> "do" GuardedCommandList "od"
GuardedCommandList -> GuardedCommand { "[]" GuardedCommand }
GuardedCommand -> Expression "->" StatementPart
Expression -> PrimaryExpression { PrimaryOperator PrimaryExpression }
PrimaryOperator -> "&" | "|"
PrimaryExpression -> SimpleExpression [ RelationalOperator SimpleExpression ]
RelationalOperator -> "<" | "=" | ">"
SimpleExpression -> ["-"] Term { AddingOperator Term }
AddingOperator -> "+" | "-"
Term -> Factor { MultiplyingOperator Factor }
MultiplyingOperator -> "*" | "/" | "\"
Factor -> Constant | VariableAccess | "(" Expression ")" | "~" Factor
VariableAccess -> VariableName [ IndexedSelector ]
IndexedSelector -> "[" Expression "]"
Constant -> Numeral | BooleanSymbol | ConstantName
BooleanSymbol -> "false" | "true" 
```

The First and Follow sets for nonterminals:
| Nonterminal         | First                                  | Follow |
|---------------------|----------------------------------------|--------|
| BooleanSymbol       | 'false' 'true'                         |        |
| Constant            | Number Name 'false' 'true'             |        |
| IndexedSelector     | '\['                                   |        |
| VariableAccess      | Name                                   |        |
| Factor              | Number Name 'false' 'true' '(' '~'     |        |
| MultiplyingOperator | '\*' '/' '\'                           |        |
| Term                | Number Name 'false' 'true' '(' '~'     |        |
| AddingOperator      | '+' '-'                                |        |
| SimpleExpression    | '-' Number Name 'false' 'true' '(' '~' |        |
| RelationalOperator  | '<' '=' '>'                            |        |
| PrimaryExpression   | '-' Number Name 'false' 'true' '(' '~' |        |
| PrimaryOperator     | '&' '\|'                                |        |
| Expression          | '-' Number Name 'false' 'true' '(' '~' |        |
| GuardedCommand      | '-' Number Name 'false' 'true' '(' '~' |        |
| GuardedCommandList  | '-' Number Name 'false' 'true' '(' '~' |        |
| DoStatement         | 'do'                                   |        |
| IfStatement         | 'if'                                   |        |
| ProcedureStatement  | 'call'                                 |        |
| AssignmentStatement | Name                                   |        |
| ExpressionList      | '-' Number Name 'false' 'true' '(' '~' |        |
| WriteStatement      | 'write'                                |        |
| VariableAccessList  | Name                                   |        |
| ReadStatement       | 'read'                                 |        |
| EmptyStatement      | 'skip'                                 |        |
| Statement           | 'skip' 'write' Name 'call' 'if' 'do'   |        |
| StatementPart       | e 'skip' 'write' Name 'call' 'if' 'do' |        |
| ProcedureDefiniton  | 'proc'                                 |        |
| VariableList        | Name                                   |        |
| TypeSymbol          | 'Integer' 'Boolean'                    |        |
| VariableDefinition  | 'Integer' 'Boolean'                    |        |
| ConstantDefinition  | 'const'                                |        |
| Definition          | 'const' 'Integer' 'Boolean' 'proc'     |        |
| DefinitionPart      | e 'const' 'Integer' 'Boolean' 'proc'   |        |
| Block               | 'begin'                                |        |
| Program             | 'begin'                                |        |

