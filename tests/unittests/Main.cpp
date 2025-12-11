#include "TestsHelper.hpp"


#include <boost/ut.hpp>
#include <fmt/format.h>
#include <fmt/chrono.h>

#include <TestsHelper.hpp>

#include <string>
#include <chrono>

int main(const int argc, char** argv)
{
    using namespace boost;

    if (argc == 2 && std::string(argv[1]) == "update")
        shouldWriteNewDiffsTofile(true);

    std::string filter = "*";
    if (argc >= 2 && std::string(argv[1]) != "update")
        filter = argv[1];

    const auto path = getResourcePath("DiagnosticsSuite/runtime");
    for (const auto& entry : std::filesystem::directory_iterator(path))
        std::cout << "# " << entry.path().generic_string() << std::endl;

    bool failed = false;
    const auto start = std::chrono::high_resolution_clock::now();

    {
        ut::cfg<ut::override> = { .filter = filter };
        failed = ut::cfg<ut::override>.run();
    }

    const auto elapsed = std::chrono::high_resolution_clock::now() - start;
    fmt::println("Tests run in {:.3%S}s", elapsed);

    return failed;
}
