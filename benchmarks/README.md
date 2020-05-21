# Benchmarks

Compiled on WSL (Ubuntu 18.04.3 LTS 64 bits), 125 runs of each test.

Running on 8 X 1992 MHz CPU
CPU Caches:
* L1 Data 32K (x4)
* L1 Instruction 32K (x4)
* L2 Unified 256K (x4)
* L3 Unified 8192K (x1)

Programs used:
* C++: g++ 8.3.0
* Java: OpenJDK 11.0.6 64 bits
* Lua: Lua 5.1.5
* Python: Python 3.6.9
* JavaScript: SpiderMonkey on Firefox 74

## Ackermann Péter test

The Ackermann function, due to its definition in terms of extremely deep recursion, can be used as a benchmark of a compiler's ability to optimize recursion.

Parameters used are m=3 and n=6.

| data   | ArkScript | C++      | Java     | Lua      | Python    | JavaScript |
| ------ | --------- | -------- | -------- | -------- | --------- | ---------- |
| mean   | 50.9 ms   | 0.152 ms | 0.152 ms | 4.750 ms | 15.334 ms | 19.76 ms   |
| median | 50.7 ms   | 0.144 ms | 0 ms     | 4.666 ms | 13.095 ms | 20 ms      |
| stddev | 1.81 ms   | 0.012 ms | 0.359 ms | 0.253 ms | 5.068 ms  | 1.046 ms   |

## List allocation test

Allocating list of 1000 elements (all numbers, only 0).

| data   | ArkScript  | C++         | Java      | Lua       | Python     | JavaScript |
| ------ | ---------- | ----------- | --------- | --------- | ---------- | ---------- |
| mean   | 0.017 ms   | 0.000618 ms | 0.016 ms  | 0.1647 ms | 0.00325 ms | 0.016 ms   |
| median | 0.016 ms   | 0.000586 ms | 0 ms      | 0.1649 ms | 0.0024 ms  | 0 ms       |
| stddev | 0.001 ms   | 0.000097 ms | 0.1254 ms | 0.0813 ms | 0.00339 ms | 0.125 ms   |