import sys
import glob
import lizard

files = glob.glob("./include/*", recursive=True) + glob.glob("./src/*", recursive=True)

UPDATED_FILES = sys.argv[1:]
MAX_CCN = 15
MAX_NLOC = 100
MAX_PARAM = 6

TABLE_HEADERS = """| Filename | Start line:end line | Function name | Parameters | NLOC | CCN |
| -------- | ------------------- | ------------- | ---------- | ---- | --- |"""

updated = []
anything_else = []

for file in lizard.analyze(files):
    for func in file.function_list:
        filename = func.filename.replace('\\', '/').lstrip('./')
        param_count = len(func.parameters)

        if func.cyclomatic_complexity >= MAX_CCN or func.nloc >= MAX_NLOC or param_count >= MAX_PARAM:
            line = [
                f"{filename} | {func.start_line}:{func.end_line} | `{func.name}` | {param_count} | {func.nloc}",
                func.cyclomatic_complexity
            ]
            if filename in UPDATED_FILES:
                updated.append(line)
            else:
                anything_else.append(line)


def make_sorted_table_lines(lines_with_ccn):
    if lines_with_ccn:
        output = TABLE_HEADERS + "\n"
        for line in sorted(lines_with_ccn, key=lambda e: e[1], reverse=True):
            output += f"| {line[0]} | {line[1]} |\n"
        return output
    return ""


print(f"""### Lizard report

Listing only functions with cyclomatic complexity >= {MAX_CCN} or NLOC >= {MAX_NLOC} or parameters >= {MAX_PARAM}.
{make_sorted_table_lines(updated)}

<details>
<summary>Report about files you didn't modify in this PR</summary>

{make_sorted_table_lines(anything_else)}
</details>
""")
