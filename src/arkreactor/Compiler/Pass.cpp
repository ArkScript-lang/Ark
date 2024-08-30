#include <Ark/Compiler/Pass.hpp>
#include <utility>

namespace Ark::internal
{
    Pass::Pass() :
        m_logger("", 0)
    {}

    Pass::Pass(std::string name, const unsigned debug_level) :
        m_logger(std::move(name), debug_level)
    {}

}
