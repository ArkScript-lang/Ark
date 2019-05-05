# Using the CLI

The Ark command line interface was designed so that you can do pretty much all the steps needed to compile, execute, and debug your Ark program in one command.

## Examples

### Compiling a file

```bash
# the outputed file will be myfile.arkc, in the same directory as myfile.ark
$ Ark myfile.ark -c
# this is also valid
$ Ark myfile.ark --compile
# setting the output path for the compiler
$ Ark myfile.ark -c -o myfile
```

### Executing a file with the interpreter

```bash
# if no options are given, the interpreter is triggered
$ Ark myfile.ark
```

### Executing a file on the virtual machine

```bash
# the file must be an Ark bytecode file
$ Ark myfile.arkc -vm
```

### Running the REPL (Read-Eval-Print Loop)

```bash
# if no arguments given, it automatically launches the REPL
$ Ark
```

## Compiling and running a program on the VM

```bash
# the VM will take 'myfile' as the Ark bytecode file to execute
$ Ark myfile.ark -c -o myfile -vm
# the VM will take the default output path of the compiler as the filename to execute (FILENAME.arkc, FILENAME being the name of the provided file)
$ Ark myfile.ark -c -vm
```

## Nota Bene

The arguments `-d|--debug` and `-t|--timer` can be passed to any of the commands, if they are at the end of the command.