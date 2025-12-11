#ifndef ARK_TESTSHELPER_HPP
#define ARK_TESTSHELPER_HPP

#include <Ark/Utils/Files.hpp>
#include <Ark/Utils/Utils.hpp>
#include <Ark/Error/Exceptions.hpp>
#include <Ark/Error/Diagnostics.hpp>

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

const auto lib_path = std::filesystem::path(ARK_TESTS_ROOT "/lib/");
const auto unittests_path = std::filesystem::path(ARK_TESTS_ROOT "/tests/unittests/");

/**
 * @brief Check if the tests should update their dataset
 * @param should used by Main.cpp to toggle on/off the updating mechanism
 * @return true if the tests should write the actual data to the 'expected' file
 */
bool shouldWriteNewDiffsTofile(std::optional<bool> should = std::nullopt);

struct TestData
{
    std::string path;      ///< The file we are testing, eg tests/unittests/resources/ASTSuite/testname.ark
    std::string stem;      ///< The stem of the path, "testname"
    std::string expected;  ///< Content of the expected file alongside the test file
    bool is_folder = false;
};

struct IterTestFilesParam
{
    const std::string expected_ext = "expected";
    bool skip_folders = true;
    bool folder_is_resource = true;
    bool ignore_expected = false;
};

void updateExpectedFile(const TestData& data, const std::string& actual);

/**
 * @brief Iterate over the files inside a folder, looking for "name.ark" & "name.expected" files to create a TestData structure
 * @param folder folder to list files in
 * @param test test function, taking a TestData&& with the paths of the input and its expected result
 * @param params optionally specify the expected extension. Defaults to "expected"
 */
void iterTestFiles(const std::string& folder, std::function<void(TestData&&)>&& test, IterTestFilesParam&& params = {});

/**
 * @brief Given an input folder, returns the resource path relatives to the project root
 * @param folder
 * @return std::string full path to the resource
 */
std::string getResourcePath(const std::string& folder);

std::string sanitizeCodeError(const Ark::CodeError& e);

std::string sanitizeRuntimeError(const std::exception& e);

void expectOrDiff(const std::string& expected, const std::string& received);

#endif  // ARK_TESTSHELPER_HPP
