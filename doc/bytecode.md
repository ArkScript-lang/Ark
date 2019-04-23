# Ark bytecode

A basic file is composed of those headers:
- magic number, 6386283 ('ark'), then a zero padding
- symbols table
    - number of elements (two bytes, big endian)
    - strings, null terminated
- values table (aka constants table)
    - number of elements (two bytes, big endian)
        - type (1 byte)
            - 0x01 for number
            - 0x02 for string
        - value
            - number: represented in hexadecimal format (stored as a null terminated string), big endian
            - string: all the characters, plus a \0 at the end (aka null terminated string)
- code segments (can have multiple code segments)
    - number of elements (two bytes, big endian), can be equal to 0
    - instructions

## Note on jumps

Jumps are used to jump from a code segment to another, in case of functions. The page number is directly encoded on two bytes (big endian).

## Instructions

| Code | Argument(s) | Job |
| ---- | ----------- | --- |
| `NOP` (0x00) | | Does Nothing |
| `LOAD_SYMBOL` (0x01) | symbol id (two bytes, big endian) | Load a symbol from its id onto the stack |
| `LOAD_CONST` (0x02) | constant id (two bytes, big endian) | Load a constant from its id onto the stack |
