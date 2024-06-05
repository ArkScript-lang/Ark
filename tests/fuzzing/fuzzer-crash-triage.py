#!/usr/bin/env python3
import sys
import os


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

    args = sys.argv[1:]
    for file in args:
        if os.path.exists(file) and os.path.isfile(file):
            print(f"{file}\n{'-' * len(file)}")
            os.system(f"{arkscript} \"{file}\"")
            command = input("\n\no (ok, not a crash), c (crash), q (quit) > ").strip().lower()
            if command == "o":
                os.rename(file, f"fct-ok/{os.path.basename(file)}")
            elif command == "c":
                os.rename(file, f"fct-bad/{os.path.basename(file)}")
            elif command == "q":
                break
            else:
                print("Unknown command. Skipping")
            print()


if __name__ == '__main__':
    main()
