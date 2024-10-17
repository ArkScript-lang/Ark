#ifndef ARK_TESTSHELPER_HPP
#define ARK_TESTSHELPER_HPP

#include <Ark/Files.hpp>
#include <Ark/Exceptions.hpp>

#include <string>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <cctype>
#include <locale>
#include <ranges>

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

std::string get_resource_path(const std::string& folder);

inline std::string& ltrim(std::string& s)
{
    s.erase(s.begin(), std::ranges::find_if(s, [](const unsigned char ch) {
                return !std::isspace(ch);
            }));
    return s;
}

inline std::string& rtrim(std::string& s)
{
    s.erase(std::ranges::find_if(s.rbegin(), s.rend(), [](const unsigned char ch) {
                return !std::isspace(ch);
            }).base(),
            s.end());
    return s;
}

std::string sanitize_error(const Ark::CodeError& e, bool remove_in_file_line = false);

#endif  // ARK_TESTSHELPER_HPP
