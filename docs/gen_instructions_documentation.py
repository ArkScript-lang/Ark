#!/usr/bin/env python3
import re


def valid(line: str) -> bool:
    return not line.startswith("enum Instruction") and not line.strip() == "{" and line.strip()


def get_comment_blob(group, blob: str):
    # search for a // @blob comment inside the group, return None if not found
    search = f"// @{blob} "
    for e in group:
        if e.startswith(search):
            return e[len(search):]
    return None


def get_doc(group):
    # the instruction is always the last element
    inst_line = group[-1]
    inst, value = inst_line.replace(",", "").split(" = ")

    return {
        "args": get_comment_blob(group, "args"),
        "role": get_comment_blob(group, "role"),
        "name": inst,
        "value": value
    }


with open("include/Ark/Compiler/Instructions.hpp") as f:
    content = f.read()
    start = content.index("enum Instruction")
    end = content.index("};")

    docs = content[start:end].split("\n")
    docs = [s.strip() for s in docs if valid(s)]

groups = []
current = []
for line in docs:
    if re.match("[A-Z0-9_]+ = 0x[0-9a-f]{2},?", line) is not None:
        current.append(line)
        groups.append(current)
        current = []
    else:
        # add doc lines
        current.append(line)

for inst in groups:
    # skip instructions with no documentation
    if len(inst) == 1:
        continue

    doc = get_doc(inst)
    print(f"""| `{doc["name"]}` ({doc["value"]}) | {doc["args"] or ""} | {doc["role"]} |""")
