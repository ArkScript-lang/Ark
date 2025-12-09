#include "TestsHelper.hpp"

#include <Ark/Utils/Utils.hpp>

#include <dtl.hpp>
#include <sstream>
#include <boost/ut.hpp>

bool shouldWriteNewDiffsTofile(const std::optional<bool> should)
{
    static bool update = false;
    if (should.has_value())
        update = should.value();

    return update;
}

void updateExpectedFile(const TestData& data, const std::string& actual)
{
    std::filesystem::path expected_path = data.path;
    expected_path.replace_extension("expected");

    std::ofstream f(expected_path.generic_string());
    if (f.is_open())
    {
        f << actual;
        f.close();
    }
}

void iterTestFiles(const std::string& folder, std::function<void(TestData&&)>&& test, IterTestFilesParam&& params)
{
    boost::ut::test(folder) = [&] {
        const auto path = params.folder_is_resource ? getResourcePath(folder) : folder;
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.path().extension() != ".ark" && params.skip_folders)
                continue;
            if (entry.path().extension() == "." + params.expected_ext && !params.skip_folders)
                continue;

            std::string expected;

            if (!params.ignore_expected)
            {
                std::filesystem::path expected_path = entry.path();
                expected_path.replace_extension(params.expected_ext);
                expected = Ark::Utils::readFile(expected_path.generic_string());
                // getting rid of the \r because of Windows
                std::erase(expected, '\r');
                Ark::Utils::rtrim(expected);
            }

            auto data = TestData {
                .path = entry.path().generic_string(),
                .stem = entry.path().stem().generic_string(),
                .expected = expected,
                .is_folder = is_directory(entry.path())
            };

            test(std::move(data));
        }
    };
}

std::string getResourcePath(const std::string& folder)
{
    return (ARK_TESTS_ROOT "tests/unittests/resources/") + folder;
}

std::string sanitizeCodeError(const Ark::CodeError& e, const bool remove_in_file_line)
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

std::string sanitizeRuntimeError(const std::exception& e)
{
    // std::replace(s.begin(), s.end(), '\\', '/');
    std::string diag = e.what();

    // because of windows
    diag.erase(std::ranges::remove(diag, '\r').begin(), diag.end());
    std::ranges::replace(diag, '\\', '/');

    // remove the directory prefix so that we are environment agnostic
    while (diag.find(ARK_TESTS_ROOT) != std::string::npos)
        diag.erase(diag.find(ARK_TESTS_ROOT), std::size(ARK_TESTS_ROOT) - 1);
    Ark::Utils::ltrim(Ark::Utils::rtrim(diag));
    // remove last line, At IP:.., PP:.., SP:..
    diag.erase(diag.find_last_of('\n'), diag.size() - 1);
    // we most likely have a blank line at the end now
    Ark::Utils::rtrim(diag);

    return diag;
}

void expectOrDiff(const std::string& expected, const std::string& received)
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
