# Ark bytecode

A basic file is composed of those headers:
- magic number, 6386283 ('ark'), then a zero padding
- Ark version used when compiling
    - major on two bytes, big endian
    - minor on two bytes, big endian
    - patch on two bytes, big endian
- timestamp (build date, 8 bytes, unix format)
- symbols table
    - number of elements (two bytes, big endian)
    - strings, null terminated
- values table (aka constants table)
    - number of elements (two bytes, big endian)
        - type (1 byte)
            - 0x01 for number
            - 0x02 for string
            - 0x03 for function
        - value
            - number: represented in hexadecimal format (stored as a null terminated string), big endian
            - string: all the characters, plus a \0 at the end (aka null terminated string)
            - function: page number (two bytes, big endian)
- plugins table
    - number of elements (two bytes, big endian)
    - strings (names of the plugins), null terminated
- code segments (can have multiple code segments)
    - number of elements (two bytes, big endian), can be equal to 0
    - instructions

## Note on builtins

Builtins are handled with `BUILTIN id`, with `id` being the id of the builtin function object. The ids of the builtins are listed below.

| ID | Arguments(s) | Job |
| -- | ------------ | --- |
| `false` (0) | | |
| `true` (1) | | |
| `nil` (2) | | |
| `append` (3) | at least 2 (first one must be a list) | Append all arguments to the first one |
| `concat` (4) | at least 2 (must be lists) | Concat all the lists into one |
| `list` (5) | at least 0 | Return a list composed of all the arguments |
| `print` (6) | at least 0 | Print the given arguments |
| `input` (7) | 0 or 1 | Can take a String (prompt), return a String writen by the user in the shell |
| `writeFile` (8) | 2 or 3 | Filename, (mode) and content |
| `readFile` (9) | 1 | Filename |
| `fileExists?` (10) | 1 | Filename |

## Instructions

| Code | Argument(s) | Job |
| ---- | ----------- | --- |
| `NOP` (0x00) | | Does Nothing |
| `LOAD_SYMBOL` (0x01) | symbol id (two bytes, big endian) | Load a symbol from its id onto the stack |
| `LOAD_CONST` (0x02) | constant id (two bytes, big endian) | Load a constant from its id onto the stack. Should check for a saved environment and push a Closure with the page address + environment instead of the constant |
| `POP_JUMP_IF_TRUE` (0x03) | absolute address to jump to (two bytes, big endian) | Jump to the provided address if the last value on the stack was equal to true. Remove the value from the stack no matter what it is |
| `STORE` (0x04) | symbol id (two bytes, big endian) | Take the value on top of the stack and put it inside a variable named following the symbol id (cf symbols table), in the nearest scope. Raise an error if it couldn't find a scope where the variable exists |
| `LET` (0x05) | symbol id (two bytes, big endian) | Take the value on top of the stack and create a constant in the current scope, named following the given symbol id (cf symbols table) |
| `POP_JUMP_IF_FALSE` (0x06) | absolute address to jump to (two bytes, big endian) | Jump to the provided address if the last value on the stack was equal to false. Remove the value from the stack no matter what it is |
| `JUMP` (0x07) | absolute address to jump to (two byte, big endian) | Jump to the provided address |
| `RET` (0x08) | | If in a code segment other than the main one, quit it, and push the value on top of the stack to the new stack ; should as well delete the current environment. Otherwise, acts as a `HALT` |
| `HALT` (0x09) | | Stop the Virtual Machine |
| `CALL` (0x0a) | number of arguments when calling the function | Call function from its symbol id located on top of the stack. Take the given number of arguments from the top of stack and give them  to the function (the first argument taken from the stack will be the last one of the function). The stack of the function is now composed of its arguments, from the first to the last one |
| `CAPTURE` (0x0b) | symbol id (two bytes, big endian) | Used to tell the Virtual Machine to capture the variable from the current environment. Main goal is to be able to handle closures, which need to save the environment in which they were created |
| `BUILTIN` (0x0c) | id of builtin (two bytes, big endian) | Push the builtin function object on the stack |
| `MUT` (0x0d) | symbol id (two bytes, big endian) | Take the value on top of the stack and create a variable in the current scope, named following the given symbol id (cf symbols table) |
| `DEL` (0x0e) | symbol id (two bytes, big endian) | Remove a variable/constant named following the given symbol id (cf symbols table) |
| `SAVE_ENV` (0x0f) | | Save the current environment, useful for quoted code |
| `GET_FIELD` (0x10) | symbol id (two bytes, big endian) | Used to read the field named following the given symbol id (cf symbols table) of a `Closure` stored in TS. Pop TS and push the value of field read on the stack |
| `ADD` (0x20) |  | Push `TS1 + TS` |
| `SUB` (0x21) |  | Push `TS1 - TS` |
| `MUL` (0x22) |  | Push `TS1 * TS` |
| `DIV` (0x23) |  | Push `TS1 / TS` |
| `GT` (0x24) |  | Push `TS1 > TS` |
| `LT` (0x25) |  | Push `TS1 < TS` |
| `LE` (0x26) |  | Push `TS1 <= TS` |
| `GE` (0x27) |  | Push `TS1 >= TS` |
| `NEQ` (0x28) |  | Push `TS1 != TS` |
| `EQ` (0x29) |  | Push `TS1 == TS` |
| `LEN` (0x2a) |  | Push `len(TS)`, TS must be a list |
| `EMPTY` (0x2b) |  | Push `empty?(TS)`, TS must be a list |
| `FIRSTOF` (0x2c) |  | Push `firstof(TS)`, the first element of TS (must be a list) |
| `TAILOF` (0x2d) |  | Push `tailof(TS)`, all the elements of TS except the first one (TS must be a list) |
| `HEADOF` (0x2e) |  | Push `headof(TS)`, all the elements of TS except the last one (TS must be a list) |
| `ISNIL` (0x2f) |  | Push true if TS is nil, false otherwise |
| `ASSERT` (0x30) |  | Throw an exception if TS1 is false, and display TS (must be a string). Otherwise, push nil |
| `TO_NUM` (0x31) |  | Convert TS to number (must be a string) |
| `TO_STR` (0x32) |  | Convert TS to string (must be a number) |
| `AT` (0x33) |  | Push the value at index TS (must be a number) in TS1 (must be a list) |
| `AND_` (0x34) |  | Push true if TS and TS1 are true, false otherwise |
| `OR_` (0x35) |  | Push true if TS or TS1 is true, false otherwise |
| `MOD` (0x36) |  | Push `TS1 % TS` |
| `TYPE` (0x37) | | Push the type of TS as a string |
| `HASFIELD` (0x38) | | Check if TS1 is a closure field of TS. TS must be a Closure and TS1 a String |

## Example

```
0x61 0x72 0x6b 0x00  # ark\0
0x00 0x03 0x00 0x01 0x00 0x00  # version 3.1.0
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00  # timestamp: 1/1/1970 at 0:00:00

0x01  # symbols table
    0x00 0x02  # 2 elements
        0x68 0x65 0x6c 0x6c 0x6f 0x00  # 'hello'
        0x77 0x6f 0x72 0x6c 0x64 0x00  # 'world'

0x02  # constants table
0x00 0x03  # 3 elements
    0x02  # string
        0x61 0x72 0x6b 0x00  # "ark"
    0x03  # function
        0x00 0x01  # page number 1
    0x01  # number
        0x31 0x2e 0x34 0x32 0x00  # string version of '1.42'

0x03  # plugins table
0x00 0x00  # no elements for this example

0x04  # code segment start
    0x00 0x10  # number of elements
        0x02  # load const
            0x00 0x01  # constant number 1, a function
        0x05  # let
            0x00 0x00  # create an immutable variable with symbol id 0, and put the top of the stack in it:
                       # the function we loaded from the constants table
        0x02  # load const
            0x00 0x02  # constant number 2, the number 1.42
        0x01  # load symbol
            0x00 0x00  # symbol number 0: 'hello'
        0x0a  # call
            0x00 0x01  # 1 argument
        0x09  # halt

    0x00 0x10  # number of elements
        0x0d  # mut
            0x00 0x01  # create mutable variable with symbol id 1 => 'world', put argument in it
                       # (it's the top of the stack)
        0x01  # load symbol
            0x00 0x01  # symbol number 1: 'world'
        0x02  # load const
            0x00 0x00  # constant number 0, "ark"
        0x0c  # builtin
            0x00 0x06  # print
        0x0a  # call
            0x00 0x02  # 2 arguments
        0x08  # ret
```

This bytecode is the exact translation of:

```clojure
{
    (let hello (fun (world) (print world "ark")))
    (hello 1.42)  # prints `1.42 ark`
}
```
