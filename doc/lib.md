# Ark standard library

## Builtins

### Basic data manipulation functions

Functions such as `+`, `-`, `*`, and `/` take only 2 arguments, like the comparison functions: `<`, `>`, `<=`, `>=`, `=` and `!=`.  
**Nota Bene**: Equality operator is written `=`, not `==`!

### Lists manipulation functions

Length of a list (1 argument): `len`.

Is the list empty? (1 argument): `empty?`.

Get the first element of a list (1 argument): `firstof`.

Get all the elements of a list, **except** the last one (1 argument): `headof`.

Get all the elements of a list, **except** the first one (1 argument): `tailof`.

Add content to a list (first argument must be a list ; at least 2 arguments): `append`.

Concat multiple lists (at least 2 arguments, all of them must be lists): `concat`.

Create a list (at least 0 argument): `list`.

Get an element from a list by its index: `@` (give the list first, then the index).

### IO

Print multiple values (at least 1 argument) with `print`.

Ask a value to the user (0 or 1 argument, a prompt of type String) with `input`, returns a String.

Read a file with `readFile`, taking a filename (String), returns a String.

Write to a file with `writeFile`, taking a filename (String), an optional mode (String: `a` (append to end), `w` (overwrite if file exists)) and a content (any type should work).

Check if a file exists with `fileExists?` taking a filename (String).

### Miscellaneous

Booleans: `true`, `false`.  
Null type: `nil`.

Test if a value is equal to `nil` (1 argument): `isnil?`.

Equivalent of `assert` in C (2 arguments, a Boolean and a String): `assert`.

Test if two values are equal to true: `and`.  
Test if at least one value out of two is equal to true: `or`.

### Conversions

Convert a String to Number (1 argument): `toNumber`.

Convert a value to String (1 argument): `toString`.
