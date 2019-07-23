# Ark standard library

## Builtins

### Basic data manipulation functions

Functions such as `+`, `-`, `*`, and `/` take only 2 arguments, like the comparison functions: `<`, `>`, `<=`, `>=`, `=` and `!=`.  
**Nota Bene**: Equality operator is written `=`, not `==`!

Example:
```clojure
(+ 1 (* 2 (- 4 (/ 12 7))))  # translates to (4 - (12 / 7)) * 2 + 1
# we can also use + for string concatenations
(print (+ "hello" " world"))  # hello world

(= 5 b)  # test if b is equal to 5
```

### Lists manipulation functions

Create a list (at least 0 argument): `list`.

Get an element from a list by its index: `@` (give the list first, then the index).

```clojure
(let L (list 1 2 3))
# a shorter notation is also available
(let M [1 2 3])

(print (@ L 2))  # 3 (index starts from 0)
```

Length of a list (1 argument): `len`.

```clojure
(let L [1 2])
(print (len L))  # 2
```

Is the list empty? (1 argument): `empty?`.

```clojure
(if (empty? L)
    (print "L is empty")
    ()
)
```

Get the first element of a list (1 argument): `firstof`.

Get all the elements of a list, **except** the last one (1 argument): `headof`.

Get all the elements of a list, **except** the first one (1 argument): `tailof`.

```clojure
(let L [1 2 3])
(print (firstof L))  # 1
(print (headof L))  # ( 1 2 )
(print (tailof L))  # ( 2 3 )
```

Add content to a list (first argument must be a list ; at least 2 arguments): `append`.

Concat multiple lists (at least 2 arguments, all of them must be lists): `concat`.

```clojure
(let L [1 2 3])
(let M [4 5 6])
(print (append L M 12))  # ( 1 2 3 ( 4 5 6 ) 12)
(print (concat L M [12]))  # ( 1 2 3 4 5 6 12)
```

### IO

Print multiple values (at least 1 argument) with `print`.

Ask a value to the user (0 or 1 argument, a prompt of type String) with `input`, returns a String.

```clojure
(print "Hello world!")
(let text (input "this is a prompt>"))
# you will be prompted `this is a prompt>`
# validate using Return
(print text)  # prints what you typed
```

Read a file with `readFile`, taking a filename (String), returns a String.

Write to a file with `writeFile`, taking a filename (String), an optional mode (String: `a` (append to end), `w` (overwrite if file exists)) and a content (any type should work).

Check if a file exists with `fileExists?` taking a filename (String).

```clojure
(let content (readFile "foo.txt"))
(print content)  # prints the content of foo.txt

(writeFile "bar.txt" content)
# append to end
(writeFile "bar.txt" "a" 12)  # we can write anything which can be printed
```

### Miscellaneous

Booleans: `true`, `false`.  
Null type: `nil`.

Test if a value is equal to `nil` (1 argument): `isnil?`.

Equivalent of `assert` in C (2 arguments, a Boolean and a String): `assert`.

```clojure
(let a true)
(let b nil)

(if (isnil? a)
    (print "a is nil")
    (print "a is NOT nil")
)

# raises an assertion error and stop the execution
# available in debug and release
(assert (= a false) "a is not false!")
```

Test if two values are equal to true: `and`.  
Test if at least one value out of two is equal to true: `or`.

```clojure
(if (and a b)
    (print "a and b are true")
    (print "a or b is false")
)
```

### Conversions

Convert a String to Number (1 argument): `toNumber`.

Convert a value to String (1 argument): `toString`.

```clojure
(let number (input "enter a number>"))
(let N (toNumber number))  # throw type error if number isn't a string
# throw an invalid argument error if it can't be converted to a number
# can throw an out of range error if number is too big

(let B (toString N))  # acts as a print, without displaying anythin
# can take any type of value
```
