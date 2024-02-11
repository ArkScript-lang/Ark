#include <boost/ut.hpp>

int main()
{
    using namespace boost;

    ut::cfg<ut::override> = { .filter = "*" };
    return ut::cfg<ut::override>.run();
}
