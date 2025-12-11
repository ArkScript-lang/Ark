#include <boost/ut.hpp>
#include <fmt/format.h>
#include <fmt/chrono.h>

#include <TestsHelper.hpp>

#include <string>
#include <chrono>

void tt(const std::string& folder, std::function<void(const std::string&)>&& run)
{
    boost::ut::test(folder) = [=] {
        for (const std::string& name : {
            "11111111111111111111111111111111111111111111111111111111",
            "2222222222222222222222222222222222222222222222222222222222",
            "2222222222222222222222222222222222222222222222222222222222",
            "333333333333333333333333333333333333333333333333333333333333"})
        {
            run(name);
        }
    };
}

boost::ut::suite<"Main"> main_suite = [] {
    using namespace boost::ut;

    tt("main", [](const std::string& name) {
        should("parametrized " + name) = [] {
            expect(true);
            expect(false);
        };
    });
};

int main(const int argc, char** argv)
{
    using namespace boost;

    if (argc == 2 && std::string(argv[1]) == "update")
        shouldWriteNewDiffsTofile(true);

    std::string filter = "*";
    if (argc >= 2 && std::string(argv[1]) != "update")
        filter = argv[1];

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
