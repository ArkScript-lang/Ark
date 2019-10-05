#include <Ark/VM/State.hpp>

#include <Ark/Constants.hpp>

namespace Ark
{
    State::State(const std::string& libdir, const std::string& filename) :
        m_libdir(libdir == "" ? ARK_STD_DEFAULT : libdir), m_filename(filename)
    {}
}