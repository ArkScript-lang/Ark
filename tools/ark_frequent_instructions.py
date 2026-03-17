#!/usr/bin/env python3

import os
import sys
import glob
from itertools import islice, tee

super_insts = [
    "LOAD_CONST_LOAD_CONST",
    "LOAD_CONST_STORE",
    "LOAD_CONST_SET_VAL",
    "STORE_FROM",
    "STORE_FROM_INDEX",
    "SET_VAL_FROM",
    "SET_VAL_FROM_INDEX",
    "INCREMENT",
    "INCREMENT_BY_INDEX",
    "INCREMENT_STORE",
    "DECREMENT",
    "DECREMENT_BY_INDEX",
    "DECREMENT_STORE",
    "STORE_TAIL",
    "STORE_TAIL_BY_INDEX",
    "STORE_HEAD",
    "STORE_HEAD_BY_INDEX",
    "STORE_LIST",
    "SET_VAL_TAIL",
    "SET_VAL_TAIL_BY_INDEX",
    "SET_VAL_HEAD",
    "SET_VAL_HEAD_BY_INDEX",
    "CALL_BUILTIN",
    "CALL_BUILTIN_WITHOUT_RETURN_ADDRESS",
    "LT_CONST_JUMP_IF_FALSE",
    "LT_CONST_JUMP_IF_TRUE",
    "LT_SYM_JUMP_IF_FALSE",
    "GT_CONST_JUMP_IF_TRUE",
    "GT_CONST_JUMP_IF_FALSE",
    "GT_SYM_JUMP_IF_FALSE",
    "EQ_CONST_JUMP_IF_TRUE",
    "EQ_SYM_INDEX_JUMP_IF_TRUE",
    "NEQ_CONST_JUMP_IF_TRUE",
    "NEQ_SYM_JUMP_IF_FALSE",
    "CALL_SYMBOL",
    "CALL_SYMBOL_BY_INDEX",
    "CALL_CURRENT_PAGE",
    "GET_FIELD_FROM_SYMBOL",
    "GET_FIELD_FROM_SYMBOL_INDEX",
    "AT_SYM_SYM",
    "AT_SYM_INDEX_SYM_INDEX",
    "AT_SYM_INDEX_CONST",
    "CHECK_TYPE_OF",
    "CHECK_TYPE_OF_BY_INDEX",
    "APPEND_IN_PLACE_SYM",
    "APPEND_IN_PLACE_SYM_INDEX",
    "STORE_LEN",
    "LT_LEN_SYM_JUMP_IF_FALSE",
    "MUL_BY",
    "MUL_BY_INDEX",
    "MUL_SET_VAL",
    "FUSED_MATH"
]

compute_super_insts_usage = sys.argv[1] == "super_insts_usage"

executable = None
for p in ["./arkscript", "cmake-build-debug/arkscript", "build/arkscript", "build/arkscript.exe"]:
    if os.path.exists(p):
        executable = p
        break
if executable is None:
    print("Couldn't find a valid arkscript executable")
    sys.exit(1)

ir = []

rosetta = glob.glob("tests/unittests/resources/RosettaSuite/*.ark")
examples = glob.glob("examples/*.ark")
for file in rosetta + examples + [
    "tests/unittests/resources/LangSuite/unittests.ark",
    "lib/std/tests/all.ark"
]:
    os.system(f"{executable} -c {file} -fdump-ir --lib './lib/;./tests/unittests/'")

    d = os.path.dirname(file)
    f = os.path.basename(file)
    path = f"{d}/__arkscript__/{f}.ir"

    if os.path.exists(path):
        with open(path) as f:
            pages = f.read().split("\n\n")
            for page in pages:
                # only keep the instruction names
                insts = [
                    i.replace("\t", "").split(" ")[0]
                    for i in page.split("\n")
                ]
                # remove the page name (page_<num>)
                ir.append(insts[1:])
        os.remove(path)


def window(iterable, size):
    iterators = tee(iterable, size)
    iterators = [islice(iterator, i, None) for i, iterator in enumerate(iterators)]
    yield from zip(*iterators)


def skip_inst_for_frequency(inst):
    return inst in super_insts or inst.startswith(".L") or inst in [
        "HALT",
        "PUSH_RETURN_ADDRESS"  # only pushes to the stack, can not be coupled to another instruction that pops
    ]


frequent = {
    2: {},
    3: {},
    4: {},
}
super_insts_freqs = {}

for page in ir:
    for pair in window(page, 4):
        pair_two = (pair[0], pair[1])
        pair_three = (pair[0], pair[1], pair[2])

        if pair[0] in super_insts:
            super_insts_freqs[pair[0]] = super_insts_freqs.get(pair[0], 0) + 1

        # if there is a label in the middle of the expression group,
        # omit it from the frequencies as this can't be optimized.
        # also skip (store, store, [_]) as we can't optimize double stores
        # (used for function argument lists)
        if not skip_inst_for_frequency(pair[0]) and not skip_inst_for_frequency(pair[1]) and \
                not (pair[0] == pair[1] and pair[1] == 'STORE'):
            count_two = frequent[2].get(pair_two, 0)
            frequent[2][pair_two] = count_two + 1

            if not skip_inst_for_frequency(pair[2]):
                count_three = frequent[3].get(pair_three, 0)
                frequent[3][pair_three] = count_three + 1

                if not skip_inst_for_frequency(pair[3]):
                    count_four = frequent[4].get(pair, 0)
                    frequent[4][pair] = count_four + 1


def print_most_freqs(data, max_percent=10):
    most = sorted(data.items(), key=lambda e: e[1], reverse=True)
    interesting = most[:(len(most) * max_percent) // 100]
    if compute_super_insts_usage:
        print("| Super Instruction | Uses in compiled code |")
        print("| ----------------- | --------------------- |")
        print("\n".join(f"| {insts} | {count} |" for (insts, count) in interesting))
    else:
        print("\n".join(f"{insts} -> {count}" for (insts, count) in interesting))

    if compute_super_insts_usage:
        threshold = 10
        for (inst, count) in most:
            if count <= threshold:
                sys.exit(1)


if not compute_super_insts_usage:
    print("Super instructions present:")
print_most_freqs(super_insts_freqs, max_percent=100)

if compute_super_insts_usage:
    sys.exit(0)

for i in (2, 3, 4):
    print(f"\nPairs of {i}:")
    print_most_freqs(frequent[i])
