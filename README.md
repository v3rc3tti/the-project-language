# The Project Language

Implementation of the Project Language compiler defined in the book "Brinch Hansen on Pascal Compilers" using C programming language.

## Lexical Analysis

## Syntax Analysis

The Project Language grammar:
```
Program -> Block "."
Block -> "begin" DefinitionPart StatementPart "end"
DefinitionPart -> { Definition ";"}
 
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

