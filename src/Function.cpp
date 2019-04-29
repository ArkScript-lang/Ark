#include <Ark/Function.hpp>

namespace Ark
{
    Function::Function(Lang::Program* prog, Lang::Node function) :
        m_procedure(function),
        m_program(prog)
    {}
}
