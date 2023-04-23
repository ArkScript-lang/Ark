@page guidelines_moduleserrorhandling Module error handling

## Definitions

_error_: a C++ exception, `std::runtime_error` or `Ark::TypeError` in this project

## Precisions

We need to have something uniform between all the modules to avoid inconsistencies.

## Handling wrong arguments count

We should use `n.size()` (n the vector of arguments passed to the module's function) to check if we had everything we needed.

The message needs to be as follows: `function name need <argument count>, <description of each of the argument>`.

Example for the function `(console:color "red")`, taking a string to change the color of the text in the terminal:
~~~~{.cpp}
if (n.size() != 1)
    throw std::runtime_error("console:color need a single argument, a string representing the color to apply");
~~~~

## Handling type errors

We should compare every value type of each value in the vector of arguments with the expected type to check we had what we wanted.

Example for the function `(console:color "red"):
~~~~{.cpp}
if (n[0].valueType() != ValueType::String)
    throw Ark::TypeError("console:color need a single argument, a string representing the color to apply");
~~~~

In case of an UserType, two checks need to be done, in this particular order (we are using boolean short circuit on the or operator):
~~~~{.cpp}
if (n[0].valueType() != ValueType::User || !n[0].usertypeRef().is<Wanted C++ Type>())
    throw Ark::TypeError("my:function need a single argument, a Wanted C++ Type representing <something>");
~~~~
