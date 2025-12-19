import glob


def exclude(file: str):
    return all(not file.startswith(path) for path in [
        "lib/",
        "thirdparties/",
        "output/",
        "fuzzing/",
        "fct-bad/",
        "fct-ok/",
        "tests/fuzzing/",
        "a.ark",
        "b.ark",
        "c.ark",
        "d.ark"
    ])


for file in glob.glob("**/*.ark", recursive=True):
    if exclude(file):
        new_filename = file.replace("/", "_").lower()
        with open(f"tests/fuzzing/corpus/{new_filename}", "w") as f:
            with open(file) as source:
                f.write(source.read())
print("Copied all *.ark files to tests/fuzzing/corpus/")
