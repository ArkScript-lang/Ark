# ArkScript integration tests

You will find various ArkScript/C++ integration tests in this folder, and this document will attempt to describe how they work.

## Compilation

Each test much be referenced in the `TARGET_LIST` of the CMakeLists, separated by a semicolon. Do *not* put the extension `.cpp` as it is automatically added.

CMake will then generate an executable target per listed file, and copy them in a special directory here, `out/`.

## The runner

You should launch it from this directory:

```shell
~/ark/tests/cpp/$ ./run-tests
TEST 01-test PASSED -- in 0.5050034sec
TEST 02-test PASSED -- in 0.274832256sec
TEST 03-test PASSED -- in 0.6026941sec

3 passed, 0 failed
```

It will loop over each executable file in `out/` and launch them one after another. The exit code will be checked, and if it is a non-zero code, the test is marked as *failed*. The output produced by the tests are also tested against the one un `expected/{test_name}.txt`. They should be identical, minus the CRLF/LF difference.