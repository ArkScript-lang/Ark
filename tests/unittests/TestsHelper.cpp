#include "TestsHelper.hpp"

#include <Ark/Utils.hpp>

#include <dtl.hpp>
#include <sstream>
#include <boost/ut.hpp>

void iter_test_files(const std::string& folder, std::function<void(TestData&&)>&& test, const std::string& expected_ext)
{
    for (const auto& entry : std::filesystem::directory_iterator(get_resource_path(folder)))
    {
        if (entry.path().extension() != ".ark")
            continue;

        std::filesystem::path expected_path = entry.path();
        expected_path.replace_extension(expected_ext);
        std::string expected = Ark::Utils::readFile(expected_path.generic_string());
        // getting rid of the \r because of Windows
        std::erase(expected, '\r');
        ltrim(rtrim(expected));

        auto data = TestData {
            .path = entry.path().generic_string(),
            .stem = entry.path().stem().generic_string(),
            .expected = expected
        };

        test(std::move(data));
    }
}

std::string get_resource_path(const std::string& folder)
{
    return (ARK_TESTS_ROOT "tests/unittests/resources/") + folder;
}

std::string sanitize_error(const Ark::CodeError& e, const bool remove_in_file_line)
{
    std::stringstream stream;
    Ark::Diagnostics::generate(e, stream, /* colorize= */ false);

    std::string diag = stream.str();
    diag.erase(std::ranges::remove(diag, '\r').begin(), diag.end());
    if (diag.find(ARK_TESTS_ROOT) != std::string::npos)
        diag.erase(diag.find(ARK_TESTS_ROOT), std::size(ARK_TESTS_ROOT) - 1);

    if (remove_in_file_line)
        diag.erase(0, diag.find_first_of('\n') + 1);

    return diag;
}

void expect_or_diff(const std::string& expected, const std::string& received)
{
    const bool comparison = expected == received;
    boost::ut::expect(comparison) << [&] {
        dtl::Diff<std::string, std::vector<std::string>> d(
            Ark::Utils::splitString(received, '\n'),
            Ark::Utils::splitString(expected, '\n'));
        d.enableHuge();
        d.compose();
        d.composeUnifiedHunks();
        std::stringstream stream;
        d.printUnifiedFormat(stream);

        return stream.str();
    };
}
