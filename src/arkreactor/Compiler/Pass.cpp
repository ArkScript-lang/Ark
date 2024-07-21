#include <Ark/Compiler/Pass.hpp>
#include <utility>

namespace Ark::internal
{
    Pass::Pass() :
        m_debug(0)
    {}

    Pass::Pass(std::string name, const unsigned debug_level) :
        m_debug(debug_level), m_name(std::move(name))
    {}

}
