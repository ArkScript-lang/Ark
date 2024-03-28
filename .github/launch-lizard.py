import glob
import lizard

files = glob.glob("./include/*", recursive=True) + glob.glob("./src/*", recursive=True)

i = lizard.analyze(files)

print(f"""### Lizard report
---

Listing only functions with cyclomatic complexity >= 15 or NLOC >= 100 or parameters >= 10.

| Filename | Start line:end line | Function name | Parameters | NLOC | CCN |
| -------- | ------------------- | ------------- | ---------- | ---- | --- |""")

data = []

for file in i:
    for func in file.function_list:
        filename = func.filename.replace('\\', '/')
        param_count = len(func.parameters)

        if func.cyclomatic_complexity >= 15 or func.nloc >= 100 or param_count >= 10:
            data.append([
                f"{filename} | {func.start_line}:{func.end_line} | `{func.name}` | {param_count} | {func.nloc}",
                func.cyclomatic_complexity
            ])

for line in sorted(data, key=lambda e: e[1], reverse=True):
    print(f"| {line[0]} | {line[1]} |")
