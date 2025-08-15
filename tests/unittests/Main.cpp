#include <boost/ut.hpp>
#include <string>

int main(const int argc, char** argv)
{
    using namespace boost;

    std::string filter = "*";
    if (argc >= 2)
        filter = argv[1];

    ut::cfg<ut::override> = { .filter = filter };
    return ut::cfg<ut::override>.run();
}
