# Ark programming language

Ark is a toy programming language, inspired from Lisp.

## Basics

`()[]{}` are all equal, even if they are different to you. You can use the ones you want, or all of them, just to organize your code more easily!

`1257756` and `-7854` are numbers. Internally, they are stored on `dozerg::HugeNumber`, which allows to store really big numbers without the usual limitation (32 or 64 bits).

`"hello world"` is a string.

### Hello world!

```
{begin
    [def message "hello world!"]
    [def sayHi! (fun (msg)
        (print msg)
    )]
    [sayHi! message]  ' display `hello world!`
}
```

## Keywords

The keywords are:

* begin, creating a block with n elements (n can be equal to 0)
* def, to define a variable
* set, to modify a variable
* if, taking 3 blocks as arguments: the condition, the if clause, and the else clause. If you don't need one of the clause (or the two of them, why not), just leave an empty block
* fun, taking 2 blocks as arguments: a list of the arguments of the function, and the body of the function
* while, taking 2 blocks as arguments: the condition and the body (can be an empty block)

## Using Ark in a project

-> [Embedidng Ark](embedding.md)

## Standart library

-> [Ark std lib](lib.md)