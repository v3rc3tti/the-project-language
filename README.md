# The Project Language

Implementation of the Project Language compiler defined in the book "Brinch Hansen on Pascal Compilers" using C programming language.

## Lexical Analysis

## Syntax Analysis

The Project Language grammar:
```
Program -> Block "."
Block -> "begin" DefinitionPart StatementPart "end"
DefinitionPart -> { Definition ";"}
Definition -> ConstantDefinition | VariableDefinition | ProcedureDefinition
ConstantDefinition -> "const" Name "=" Constant
VariableDefinition -> TypeSymbol ( VariableList | "array" ArrVarList )
ArrVarList -> Name ("," ArrVarList | "[" Constant "]")
TypeSymbol -> "Integer" | "Boolean"
VariableList -> Name { "," Name }
ProcedureDefinition -> "proc" Name Block
StatementPart -> { Statement ";" }
Statement -> EmptyStatement | ReadStatement | WriteStatement | AssignmentStatement | ProcedureStatement | IfStatement | DoStatement
EmptyStatement -> "skip"
ReadStatement -> "read" VariableAccessList
VariableAccessList -> VariableAccess { "," VariableAccess }
WriteStatement -> "write" ExpressionList
ExpressionList -> Expression { "," Expression }
AssignmentStatement -> VariableAccessList ":=" ExpressionList
ProcedureStatement -> "call" Name
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
Factor -> Numeral | BooleanSymbol | VariableAccess | "(" Expression ")" | "~" Factor
VariableAccess -> Name [ IndexedSelector ]
IndexedSelector -> "[" Expression "]"
Constant -> Numeral | BooleanSymbol | Name
BooleanSymbol -> "false" | "true" 
```

The First and Follow sets for nonterminals:
| Nonterminal         | First                                        | Follow                                                            |
|---------------------|----------------------------------------------|-------------------------------------------------------------------|
| BooleanSymbol       | 'false' 'true'                               | '<' '=' '>' '&' '\|' ',' ';' '\->' '\]' '\)' '+' '-' '\*' '/' '\\'|
| Constant            | Number Name 'false' 'true'                   | '<' '=' '>' '&' '\|' ',' ';' '\->' '\]' '\)' '+' '-' '\*' '/' '\\'|
| IndexedSelector     | '\['                                         | '<' '=' '>' '&' '\|' ',' ';' '\->' '\]' '\)' '+' '-' '\*' '/' '\\'|
| VariableAccess      | Name                                         | '<' '=' '>' '&' '\|' ',' ';' '\->' '\]' '\)' '+' '-' '\*' '/' '\\'|
| Factor              | Number Name 'false' 'true' '(' '~'           | '<' '=' '>' '&' '\|' ',' ';' '\->' '\]' '\)' '+' '-' '\*' '/' '\\'|
| MultiplyingOperator | '\*' '/' '\\'                                | Number Name 'false' 'true' '(' '~'                                |
| Term                | Number Name 'false' 'true' '(' '~'           | '<' '=' '>' '&' '\|' ',' ';' '\->' '\]' '\)' '+' '-'              |
| AddingOperator      | '+' '-'                                      | Number Name 'false' 'true' '(' '~'                                |
| SimpleExpression    | '-' Number Name 'false' 'true' '(' '~'       | '<' '=' '>' '&' '\|' ',' ';' '\->' '\]' '\)'                      |
| RelationalOperator  | '<' '=' '>'                                  | '-' Number Name 'false' 'true' '(' '~'                            |
| PrimaryExpression   | '-' Number Name 'false' 'true' '(' '~'       | '&' '\|' ',' ';' '\->' '\]' '\)'                                  |
| PrimaryOperator     | '&' '\|'                                     | '-' Number Name 'false' 'true' '(' '~'                            |
| Expression          | '-' Number Name 'false' 'true' '(' '~'       | ',' ';' '\->' '\]' '\)'                                           |
| GuardedCommand      | '-' Number Name 'false' 'true' '(' '~'       | '\[\]' 'fi' 'od'                                                  |
| GuardedCommandList  | '-' Number Name 'false' 'true' '(' '~'       | 'fi' 'od'                                                         |
| DoStatement         | 'do'                                         | ';'                                                               |
| IfStatement         | 'if'                                         | ';'                                                               |
| ProcedureStatement  | 'call'                                       | ';'                                                               |
| AssignmentStatement | Name                                         | ';'                                                               |
| ExpressionList      | '-' Number Name 'false' 'true' '(' '~'       | ';'                                                               |
| WriteStatement      | 'write'                                      | ';'                                                               |
| VariableAccessList  | Name                                         | ';' ':='                                                          |
| ReadStatement       | 'read'                                       | ';'                                                               |
| EmptyStatement      | 'skip'                                       | ';'                                                               |
| Statement           | 'skip' 'write' Name 'call' 'if' 'do' 'read'  | ';'                                                               |
| StatementPart       | e 'skip' 'write' Name 'call' 'if' 'do' 'read'| 'end' '\[\]' 'fi' 'od'                                            |
| ProcedureDefinition | 'proc'                                       | ';'                                                               |
| VariableList        | Name                                         | '\[' ';'                                                          |
| TypeSymbol          | 'Integer' 'Boolean'                          | 'array' Name                                                      |
| VariableDefinition  | 'Integer' 'Boolean'                          | ';'                                                               |
| ConstantDefinition  | 'const'                                      | ';'                                                               |
| Definition          | 'const' 'Integer' 'Boolean' 'proc'           | ';'                                                               |
| DefinitionPart      | e 'const' 'Integer' 'Boolean' 'proc'         | 'end' 'skip' 'write' 'read' Name 'call' 'if' 'do'                        |
| Block               | 'begin'                                      | '.' ';'                                                           |
| Program             | 'begin'                                      | EOF                                                               |

