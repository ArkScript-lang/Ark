#!/usr/bin/env python3

import sys
import glob
import subprocess


def main():
    errors = {
        "bad_variant_access": [],
        "std::bad_alloc": [],
        "unknown instruction": [],
        "warning unused quote expression": [],
        "warning statement has no effect": [],
        "warning ignoring return value of function": [],
        "unbound variable:": [],
        "unrecognized macro form": [],
        "can not modify a constant list": [],
        "was of type": [],
        "was not provided": [],
        "but it received": [],
        "parseerror:": [],
        "compilationerror": [],
        "timeout": [],
        " should be ": [],
    }
    unclassified = []

    for i, file in enumerate(glob.glob("fuzzing/output/*/crashes/id*")):
        try:
            cmd = subprocess.run(
                ["build/arkscript", file, "-L", "./lib"],
                capture_output=True,
                check=False,  # do not check return code
                timeout=2  # 2 seconds timeout
            )
            output = cmd.stderr.decode(errors="ignore") + cmd.stdout.decode(errors="ignore")
        except subprocess.TimeoutExpired:
            output = "timeout"

        for name, values in errors.items():
            if name in output.lower():
                values.append(cmd)
                break
        else:
            unclassified.append(cmd)

    errors["unclassified"] = unclassified

    for name, values in errors.items():
        print(f"{name}: {len(values)}")

    for name in ["std::bad_alloc", "unknown instruction", "parseerror:", "compilationerror", "timeout", "unclassified"]:
        with open(f"fuzzing/output/{name.replace(':', '').replace(' ', '_')}.list", "w", encoding="utf-8") as f:
            for line in errors[name]:
                path = line.args[1]
                f.write(f"{path}\n")

    return 0


if __name__ == "__main__":
    sys.exit(main())
