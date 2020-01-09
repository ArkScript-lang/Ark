# Console module

A module made to manipulate the console.

The library used to manipulate the colors of the console is [termcolor](https://github.com/ikalnytskyi/termcolor) (BSD 3-clause License).

## Functions

### consoleClear

Clear the content of the console.

Example:

```clojure
(consoleClear)
```

### consoleColor

Set the background/foreground color of the shell. Calls can be stacked.

```clojure
{
    (consoleColor "bold")
    (consoleColor "green")
    (consoleColor "on_yellow")

    (print "hello")  # will print a bold green text on yellow on Linux
                     # only a green text on yellow on Windows which doesn't support bold text
    
    (consoleColor "reset")  # reset all attributes and colors
}
```

List of arguments available:

name | type
---- | ----
reset | reset all attributes and colors previously sent
bold | attribute
dark | attribute
underline | attribute
blink | attribute
reverse | attribute
concealed | attribute
grey | foreground
red | foreground
green | foreground
yellow | foreground
blue | foreground
magenta | foreground
cyan | foreground
white | foreground
on_grey | background
on_red | background
on_green | background
on_yellow | background
on_blue | background
on_magenta | background
on_cyan | background
on_white | background

*Nota bene*: so far, attribute manipulators aren't supported on Windows