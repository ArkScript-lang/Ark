#include <boost/ut.hpp>
#include <fmt/format.h>
#include <fmt/chrono.h>

#include <string>
#include <chrono>

int main(const int argc, char** argv)
{
    using namespace boost;

    std::string filter = "*";
    if (argc >= 2)
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
