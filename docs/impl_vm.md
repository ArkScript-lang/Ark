@page impl_vm Virtual Machine implementation details

## Constructing a VM

This is done by calling the constructor of the VM, passing in a reference to an `Ark::State`. It won't be modified, it was made this way to be able to share a single state between multiple independant virtual machines.

## Running the VM (and consequently running ArkScript bytecode)

### Initializing the VM

The first step before running is to initialize the virtual machine state, this is done each time you call `VM::run` to reset its state (stack pointer, stack frame count, shared libs, locals, scopes) and reload the binded functions.

### Running code while taking care of exceptions

This is the next function called by `VM::run`: `VM::safeRun`. As its name suggests, it runs bytecode in a safe manner, taking care of the potential exceptions. This is by far the longest method (952 lines at the time writing). It will go through every instruction, running them sequentially, and if an exception is thrown, it will be caught, the exit code of the VM will be set to 1, and a backtrace will be displayed.

### Backtrace generation

This is done by `VM::backtrace` when an exception was thrown while the VM was running. It will display at most the last 7 elements in the call stack, with the function name (if any) and the arguments with their value.
