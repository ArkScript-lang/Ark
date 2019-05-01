# Ark standard library

## Builtins

### Basic data manipulation functions

Functions such as `+`, `-`, `*`, and `/` can take multiple arguments, at least 2 are needed.

Comparisons functions are `<`, `>`, `<=`, `>=`, `=` and `!=`, they take only 2 arguments.  
**Nota Bene**: Equality operator is written `=`, not `==` !

### Lists manipulation functions

Length of a list (1 argument): `len`.  
Is the list empty? (1 argument): `empty?`.  
Get the first element of a list (1 argument): `firstof`.  
Get all the elements of a list, **except** the first one (1 argument): `tailof`.  
Add content to a list (first argument must be a list ; at least 2 arguments): `append`.  
Concat multiple lists (at least 2 arguments, all of them must be lists): `concat`.  
Create a list (at least 0 argument): `list`.

### IO

Print multiple values (at least 1 argument) with `print`.

### Miscellaneous

Booleans: `true`, `false`.  
Null type: `nil`.

Test if a value is equal to `nil` (1 argument): `isnil?`.  
Equivalent of `assert` in C (2 arguments, a Boolean and a String): `assert`.