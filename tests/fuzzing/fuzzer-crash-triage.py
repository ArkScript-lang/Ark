#!/usr/bin/env python3
import sys
import os

arkscript = None
if os.path.exists("cmake-build-debug/arkscript"):
    arkscript = "cmake-build-debug/arkscript"
elif os.path.exists("build/arkscript"):
    arkscript = "build/arkscript"
else:
    print("Couldn't find a suitable arkscript executable")
    sys.exit(1)

if not os.path.exists("fct-ok"):
    os.mkdir("fct-ok")
if not os.path.exists("fct-bad"):
    os.mkdir("fct-bad")


def run_file(file: str):
    print(f"{file}\n{'-' * len(file)}")
    exit_code = os.system(f"{arkscript} \"{file}\"")
    print(f"\nEXITCODE: {exit_code}\n\n")


def handle_command(i: int, count: int, file: str):
    command = input(f"[{i}/{count}] o,c,r,x,s,q,? > ").strip().lower()
    if command == "o":
        os.rename(file, f"fct-ok/{os.path.basename(file)}.ark")
    elif command == "c":
        basename = os.path.basename(file).split(".")[0]
        os.rename(file, f"fct-bad/{basename}.ark")
    elif command == "r":
        run_file(file)
        return True
    elif command == "x":
        with open(file) as f:
            print(f.read())
        return True
    elif command == "s":
        pass
    elif command == "q":
        sys.exit(0)
    elif command == "?":
        messages = [
            "o: ok, not a crash",
            "c: crash",
            "r: run again",
            "x: show source code",
            "s: skip",
            "q: quit",
            "?: this help message"]
        print("\t" + "\n\t".join(messages))
        return True
    else:
        print("Unknown command.")
        return True
    return False


def main():
    args = [f for f in sys.argv[1:] if "__arkscript__" not in f]
    files_count = len(args)

    if files_count == 0:
        print("No input files. Usage: script.py folder/*.ark")
        exit(1)

    for i, file in enumerate(args):
        if os.path.exists(file) and os.path.isfile(file):
            run_file(file)
            while handle_command(i + 1, files_count, file):
                pass
            print()


if __name__ == '__main__':
    main()
