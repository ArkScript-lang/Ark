#ifndef TESTS_CPP_TESTS_HPP
#define TESTS_CPP_TESTS_HPP

#include <iostream>

#define CHECK_VM_RUN(vm)                                   \
    if (auto code = vm.run(); code != 0) {                 \
        std::cerr << "vm.run() returned " << code << "\n"; \
        return 1;                                          \
    }

#define CHECK_VALUE_NUMBER(val, expected)                                                       \
    if (val.valueType() != Ark::ValueType::Number || val.number() != (expected)) {              \
        std::cerr << #val << " isn't a Number or doesn't have the value " << #expected << "\n"; \
        return 1;                                                                               \
    }

#define RETURN_PASSED() return 0;

#endif
