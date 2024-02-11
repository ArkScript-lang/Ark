#ifndef ARK_TESTSHELPER_HPP
#define ARK_TESTSHELPER_HPP

#include <Ark/Files.hpp>

#include <string>
#include <algorithm>
#include <functional>
#include <filesystem>

#ifndef ARK_TESTS_ROOT
#    define ARK_TESTS_ROOT ""
#endif

struct TestData
{
    std::string path;      ///< The file we are testing, eg tests/unittests/resources/ASTSuite/testname.ark
    std::string stem;      ///< The stem of the path, testname
    std::string expected;  ///< Content of the expected file alongside the test file
};

void iter_test_files(const std::string& folder, std::function<void(TestData&&)>&& test, const std::string& expected_ext = "expected");

#endif  // ARK_TESTSHELPER_HPP
