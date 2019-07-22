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

### Executing a file on the virtual machine

```bash
# the file must be an Ark bytecode file
$ Ark myfile.arkc -vm
```

## Nota Bene

The arguments `-d|--debug` and `-t|--timer` can be passed to any of the commands, if they are at the end of the command.
