@page impl_compiler ArkScript's compiler implementation details

The compiler is divided in multiple parts, interacting with other in sequence:
* the lexer, generating a list of tokens from source code
* the parser, generating the AST from tokens
* the macro processor, applying macros in the AST
* the optimizer, removing unused symbols from the AST
* the compiler, generating bytecode from the AST

There exists another compiler, the json compiler, converting a given code to JSON by iterating through the AST. It follows the same process as the compiler to generate the AST.

## Lexer

Its role is quite simple: following a small set of rules, it will cut through a given input (the source code) in tokens:

~~~~{.cpp}
struct Token
{
    TokenType type;
    std::string token;
    std::size_t line;
    std::size_t col;
}
~~~~

A token has a type, to help the parser when generating the AST. the type can be one of
* Grouping: `()[]{}`
* String: `"..."`
* Number
* Operator
* Identifier: a variable name
* Capture: a capture used in functions: `&identifier`
* GetField: a field reading operation: `.field` (prefixed by the closure which you are trying to read a field from)
* Keyword
* Skip: a space or newline character
* Comment: `#...\n`
* Shorthand: `'` (quote)
* Spread: `...`, used by macros
* Mismatch: anything we couldn't put in a category

The line and col values are here only for debugging purpose, when creating the error messages so that we know which line and word to point out to the user.

## Parser

It is generating the abstract syntax tree (AST) from the list of tokens, and importing files when it can, merging their AST with the AST it holds.

It is recursive, quite complex, but gets the job done (I really hope no one wants to contribute to it because I may be the only one able to understand it, but it's really hard to explain). Basically, it will eat through the tokens, and try to match them with what it think the current node must be. For example, if the parser finds a `(`, then a keyword token with the value `if`, it will attempt to build a `if` node with a condition, an `ifTrue` node, and a potential `ifFalse` node, then return the node. To generate the subnodes, it's calling itself, and will expect to find a closing `)` after returning.

## Macro processor

Once the AST has been built, the macro processor kicks into action and goes over the AST, trying to find any macro node to register them and apply them. Macros are registered by block, so if we are at a current depth of 0 in the AST, and found a macro, it will available in every node. If we find a macro in a node at depth 2, the macro will get registered in applied in all the potential subnodes of the current node, but once we leave the depth 2 node, the macro will be removed from memory and won't be accessible in the other part of the script.

~~~~{.lisp}
!{macro 0}  # available everywhere

(begin
    # macro exists here in this depth 1 node

    !{a 2}  # a exists here in this depth 1 node
)
# a doesn't exist here in this depth 0 node
~~~~

It is composed of different pieces:
* the processor, receiving the AST, keeping track of all the macros, and visiting every node, applying macros in them (it also does complex stuff with function macros like unifying the argument lists)
* the pipeline, holding all the different macro executors, trying to apply them one after another (until one matches) when the processor asks for a macro to be applied on a node
* the executors:
    * the conditionnal one works only with if macros and apply them regarding the value of the condition
    * the symbol one applies the value held by the macro
    * the list one applies predefined macros and user defined macros (function macros), it is arguably the most complex one but also the best one as well

Once it is done running, every macro definition and macro usage in the AST has been removed and/or replaced with the corresponding value, so we are left with an AST that the compiler could already work with.

## AST optimizer

Though before being able to compile the AST, we can optimize it a bit, by removing unused variable definition. For simple variables, like `(let a 12)`, this isn't really a problem if we leave them, but for functions it would create separate pages of bytecode for them, and the resulting bytecode would be a lot longer than just a variable declaration and assignment. Having a long bytecode isn't a problem in term of computing power, as we are only computing what we are asked to, but we are limited to 65'534 functions as our code segment indexes are on 2 bytes, and we are limited to 65'536 different symbols (and values). Those symbols and values could be used only in those functions we want to delete, and use substantial space in the symbols and values table.

So here comes the AST optimizer, reading only the first level of the AST, searching for variables and functions definition. Then, it visits the whole AST, counting how many times each gathered symbol has been used. In the end, if a symbol has been used only once (it correspond to its definition/declaration), then it gets removed from the AST.

## Compiler

The compiler's role is to take a tree-like structure, the AST, and flatten it, so that the virtual machine can just read one instruction after another, without having to dive in a nested structure (which is very inefficient).

It works recursively on the nodes, and sometimes even I have some trouble understanding why something works and why something else doesn't. In the end, it all boils down to "the bytecode has been badly generated".

- The first step is to generate the file header: the string `ark\0`, followed by the version of the compiler (each number on two bytes), and then the timestamp (`Compiler::pushFileHeader`).
- Next step is to run through the AST, running each node in the `Compiler::compile` recursive method, in charge of generating the bytecode.
- Then we check for any undefined symbols and display an error to the user if we found any (alongside a suggestion) (`Compiler::checkForUndefinedSymbols`)
- We can continue to build our bytecode file header, with the symbols and the values table, respectively for a list of identifier used throughout the program, and for all the values used (a tag-value system is used here so that the VM knows what type the value is) (`Compiler::pushSymAndValTables`).
- Finally, we add the different code pages to the bytecode file, generated by the `Compiler::compile` method called earlier, and create the SHA256 used for integrity checking of the bytecode, in a spot in the file header.

### The compilation

Given an input node, we make a decision based on its type (`Compiler::_compile`):
- if it's a symbol we will compile it to a `LOAD <symbol>`
- "get field operation", `GET_FIELD <symbol>`
- String, Number or nil, we push the value on the stack with `LOAD_CONST <const>`
    - an additional step is done here to avoid pushing the value on the stack **if** the value isn't being used (eg in a function call, variable assignment)
- Keywords:
    - If: first we compile the condition, then add a jump if true to go to the `ifTrue` branch. We compile the `ifFalse` branch, add a jump to the end of the if, and finally we compile the `ifTrue` branch.
    - Let, Mut, Set: we register the symbol, compile the value, add the corresponding instruction after it: `LET`, `MUT`, `STORE`
    - Fun: TODO
    - Begin: we take each node and compile them. An additional step is done here, to remove any unused value, except for the last node whose fate is decided by the caller (so that we don't always drop the last node in a begin, which may be the return value of a function).
    - While: much like the if, we compile the condition, add a jump to the end of the while if the condition is false, compile the body and add a jump to the begining (right before the condition).
    - Import: this is used only to import modules at runtime, we put the module name (a string) on the stack and then the `IMPORT` instruction. The VM will do what's needed for us.
    - Quote: we create a separate code page (corresponding to a new function), we compile the value on this new page, and add a `RET` instruction. This is arguably not your typical `quote` and I'm very sorry for this.
    - Del: this adds `DEL <symbol>` to the bytecode, to tell the virtual machine to remove `<symbol>` from memory.
- if none the previous cases matched, then we most likely have a function call (`Compiler::handleCalls`)
    - we compile the expression on a temporary bytecode page, and check its length: if it's more than 1 instruction, this is a function call, otherwise it's an operator call (which we can optimize / unroll)
    - then we compile the arguments on the current page, marking them as "no discard"
    - the tail call optimization is handled here if applicable
    - we push the computed expression for the function call and discard the temporary page
    - we add the `CALL` instruction, the number of arguments

## JSON Compiler

Given an input node, we generate the corresponding JSON structure:
- Symbol: `{"type": "Symbol", "name": "..."}`
- Capture: `{"type": "Capture", "name": "..."}`
- GetField: `{"type": "GetField", "name": "..."}`
- String: `{"type": "String", "value": "..."}`
- Number: `{"type": "Number", "value": ...}`
- Function call: `{"type": "FunctionCall", "name": "...", "args": [{"type": "Symbol", "name": "..."} ...]}`
- Keywords:
    - Function declaration: `{"type": "Fun", "args": [{"type": "Symbol", "name": "..."} ...], "body": [...] | {...}}`
    - Variable declaration/modification: `{"type": "Let|Mut|Set", "name": {"type": "Symbol", "name": "..."}, "value": {...}}`
    - Condition: `{"type": "If", "condition": [...], "then": [...] | {}, "else": [...] | {}}`
    - While loop: `{"type": "While", "condition": [...], "body": [...] | {}}`
    - Begin: `{"type": "Begin", "children": [...]}`
    - Import: `{"type": "Import", "value": {"type": "String", "value": "..."}}`
    - Quote: `{"type": "Quote", "value": [...] | {}}`
    - Del: `{"type": "Del", "value": {"type": "Symbol", "name": "..."}}`

Some fields have been filled to give you an idea of what the `value` should look like in most cases. One should note that function arguments can have `Capture` nodes as well as `Symbol` nodes.

Generic structure:
~~~~{.json}
{
    "type": "<name>",
    "value": ... || "..." || {...},  // optional
    "name": "...",  // optional
    "args": [{...}, ...],  // optional
    "condition": [{...}, ...] || {...},  // optional
    "then": [{...}, ...] || {...},  // optional
    "else": [{...}, ...] || {...},  // optional
    "body": [{...}, ...] || {...},  // optional
    "children": [{...}, ...]  // optional
}
~~~~
