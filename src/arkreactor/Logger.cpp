#include <Ark/Logger.hpp>

#include <utility>

namespace Ark::internal
{
    Logger::Logger(std::string name, const unsigned debug_level) :
        m_debug(debug_level), m_name(std::move(name))
    {}
}
