#include <Ark/Utils/Logger.hpp>

#include <array>
#include <utility>

namespace Ark::internal
{
    constexpr std::array colors = {
        fmt::color::beige,
        fmt::color::chartreuse,
        fmt::color::coral,
        fmt::color::cornflower_blue,
        fmt::color::khaki,
        fmt::color::dark_olive_green,
        fmt::color::dark_orange,
        fmt::color::dark_salmon,
        fmt::color::fire_brick,
        fmt::color::forest_green,
        fmt::color::honey_dew,
        fmt::color::medium_orchid,
        fmt::color::medium_turquoise,
        fmt::color::peru,
        fmt::color::sea_green,
        fmt::color::tomato,
        fmt::color::wheat,
        fmt::color::sea_shell
    };

    Logger::Logger(std::string name, const unsigned debug_level) :
        m_debug(debug_level), m_name(std::move(name)), m_stream(nullptr)
    {
        m_pass_color = colors[std::hash<std::string> {}(m_name) % colors.size()];
    }
}
