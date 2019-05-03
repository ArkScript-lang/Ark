# Ark programming language

Ark is a toy programming language, inspired from Lisp. The language is dynamically and strongly typed, this means that a variable can not be implicitely converted to another type, and that any variable can hold any type.

## Basics

Blocks are surrounded by `()`. Creating a block with `[...]` will expand to `(list ...)`, and creating one with `{...}` will expand to `(begin ...)`.

`1257756`, `-7854` and `-1/2` are numbers. Internally, they are stored on `Ark::BigNum`, which allows to store really big numbers without the usual limitation (32 or 64 bits).

`"hello world"` is a string.

A comment starts with `'`.

Instead of calling a function as you would do in C or Python, in Ark we use the Polish notation, so `f(a, b, c, ...)` becomes `(f a b c ...)`.

### Hello world!

```
{
    (let message "hello world!")
    (let sayHi! (fun (msg)
        (print msg)
    ))
    (sayHi! message)  ' display `hello world!`
}
```

## Creating a variable

A variable is available in the lexical scope it was created in. In C-like syntax:

```c
void fun()
{
    int x = 5;

    void fun2()
    {
        printf("%d", x);
    }
}
```

Every inner level can access its outer levels.

To define a new variable can be defined using `let` keyword, taking a symbol and a value:

```clojure
(let a 10)  ' valid definition
(let d 1 2)  ' invalid definition
(let c) ' invalid definition
```

To modify a variable, we can't just use `let` again, we should use `set`:

```
(set a 12)  ' valid
(set b 1 2)  ' invalid
(set c)  ' invalid
```

## Conditions

A condition can be invoked using `if` keyword, taking 3 nodes: a condition, a if block, and a else block.

```clojure
(if (= a 10)
    (print "a = 10")  ' then
    (print "a != 10") ' else
)
```

## Blocks

Sometimes we need to have a single block doing multiple things. We could create a function but it's too heavy, so we create blocks:

```clojure
(if (= a 10)
    ' then
    (begin
        (set a 12)
        (print "because a = 10, I changed it to 12")
    )

    ' else
    (print "nope")
)
```

## Loops

They can be summoned with `while`, taking two nodes: a condition and a "then" block.

```clojure
(while (< a 10)
    (begin
        (set a (+ a 1))
        (print a)
    )
)
```

## Functions and closures

We can create a function with the keyword `fun`. It needs 2 nodes: an argument list, and the body.

```clojure
(fun (a b c) (print a b c))
```

When creating a function, it's capturing the environment it was created in, and copy it to save it. We create what we call "closures".

```clojure
(let make-closure (fun (name)
    (fun ()
        (print name)
    )
))

' instanciating a closure
(let say-hi (make-closure "hi"))
' calling our closure
(say-hi)  ' prints `hi`
```

## Importing code

To import an Ark file into your code you can do this: `(import "myfile.ark")`. `import` can even take multiple arguments, so `(import "file1.ark" "file2.ark")` will work as well! The function needs a path relative to the location of the code importing the Ark files.

If you need to import an Ark file from the standard library, `import` will be able to find it, no need to create a relative path to the lib! Just note that all the files in the standard library have a name starting with an uppercase letter.

## Using Ark in a project

-> [Embedding Ark](embedding.md)

## Standard library

-> [Ark std lib](lib.md)

## The bytecode

-> [Ark bytecode format](bytecode.md)