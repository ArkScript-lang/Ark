#include <Ark/Utils.hpp>

namespace Ark::Utils
{
    int decPlaces(double d)
    {
        constexpr double precision = 1e-7;
        double temp = 0.0;
        int decimal_places = 0;

        do
        {
            d *= 10;
            temp = d - static_cast<int>(d);
            decimal_places++;
        } while(temp > precision && decimal_places < std::numeric_limits<double>::digits10);

        return decimal_places;
    }

    int digPlaces(double d)
    {
        int digit_places = 0;
        int i = static_cast<int>(d);
        while (i != 0)
        {
            digit_places++;
            i /= 10;
        }
        return digit_places;
    }
}