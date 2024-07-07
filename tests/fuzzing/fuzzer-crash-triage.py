#!/usr/bin/env python3
import sys
import os


def handle_command(i: int, count: int, file: str):
    command = input(f"[{i}/{count}] o,c,s,q,? > ").strip().lower()
    if command == "o":
        os.rename(file, f"fct-ok/{os.path.basename(file)}")
    elif command == "c":
        os.rename(file, f"fct-bad/{os.path.basename(file)}")
    elif command == "s":
        pass
    elif command == "q":
        sys.exit(0)
    elif command == "?":
        print("\to: ok, not a crash\n\tc: crash\n\ts: skip\n\tq: quit\n\t?: this help message")
        return True
    else:
        print("Unknown command.")
        return True
    return False


def main():
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

    args = [f for f in sys.argv[1:] if "__arkscript__" not in f]
    files_count = len(args)

    for i, file in enumerate(args):
        if os.path.exists(file) and os.path.isfile(file):
            print(f"{file}\n{'-' * len(file)}")
            os.system(f"{arkscript} \"{file}\"")
            print("\n\n")
            while handle_command(i + 1, files_count, file):
                pass
            print()


if __name__ == '__main__':
    main()
