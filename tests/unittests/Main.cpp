#include <boost/ut.hpp>

int main(int argc, const char** argv)
{
    using namespace boost;
    using namespace boost::ut::literals;
    using namespace boost::ut::operators;

    ut::expect((argc == 2_i) >> ut::fatal) << "Not enough parameters!";
    ut::cfg<ut::override> = { .filter = argv[1] };
    return ut::cfg<ut::override>.run();
}
