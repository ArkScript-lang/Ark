import pexpect
import sys
import os


def get_repl():
    executable = None
    for p in ["./arkscript", "cmake-build-debug/arkscript", "build/arkscript", "build/arkscript.exe"]:
        if os.path.exists(p):
            executable = p
            break
    if executable is None:
        print("Couldn't find a valid path to 'arkscript' executable")
        sys.exit(1)

    child = pexpect.spawn(executable, timeout=2, encoding="utf-8")
    child.logfile = sys.stdout
    return child


def test(prompt: str, command: str, expected: str):
    try:
        repl.expect(prompt)
        repl.send(command)
        if expected is not None:
            repl.expect(expected)
    except pexpect.ExceptionPexpect:
        print(f"Tried to match '{prompt}{command.strip()}' with '{expected}' but got {repl.after}")
        raise


repl = get_repl()

# repl headers
repl.expect(r"ArkScript REPL -- Version [0-9]+\.[0-9]+\.[0-9]+-(pre-)?[a-f0-9]{8} \[LICENSE: Mozilla Public License 2\.0\]")
repl.expect(r"Type \"quit\" to quit\. Try \"help\" for more information")

# completion
test("main:001> ", "(le", "let")
repl.send("t a 5)\r\n")

test("main:002> ", "(print a)\r\n", "5")

test("main:003> ", "(im", "port")
repl.sendcontrol("c")

test("main:003> ", "(let foo (fun (bar) (* bar 7))) (print (foo 7))\r\n", "49")

test("main:004> ", "# just a comment\r\n", "main:005> ")

if repl.isalive():
    repl.sendline("quit")
    repl.close()
