#include "TestsHelper.hpp"

void iter_test_files(const std::string& folder, std::function<void(TestData&&)>&& test, const std::string& expected_ext)
{
    for (const auto& entry : std::filesystem::directory_iterator((ARK_TESTS_ROOT "tests/unittests/resources/") + folder))
    {
        if (entry.path().extension() != ".ark")
            continue;

        std::filesystem::path expected_path = entry.path();
        expected_path.replace_extension(expected_ext);
        std::string expected = Ark::Utils::readFile(expected_path.generic_string());
        // getting rid of the \r because of Windows
        expected.erase(std::remove(expected.begin(), expected.end(), '\r'), expected.end());

        auto data = TestData {
            .path = entry.path().generic_string(),
            .stem = entry.path().stem().generic_string(),
            .expected = expected
        };

        test(std::move(data));
    }
}