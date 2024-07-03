import sys

UPDATED_FILES = sys.argv[1:]
TABLE_HEADER = """| Filename | Line | Type | Description |
| -------- | ---- | ---- | ----------- |"""

with open("cppcheck.txt") as f:
    content = f.readlines()

updated = []
anything_else = []

for line in content:
    filename, row, kind, desc = line.split(":", 3)
    formatted = f"| {filename} | {row} | {kind.strip()} | {desc.strip()} |"

    if filename == "nofile":
        continue

    if filename in UPDATED_FILES:
        updated.append(formatted)
    else:
        anything_else.append(formatted)


def make_output(data):
    if data:
        output = TABLE_HEADER + "\n"
        output += "\n".join(data)
        return output
    return ""


print(f"""### CppCheck report

{make_output(updated)}

<details>
<summary>Report files about files you didn't modify in this PR</summary>

{make_output(anything_else)}
</details>
""")
