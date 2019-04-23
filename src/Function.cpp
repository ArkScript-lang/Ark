#include <Ark/Function.hpp>

namespace Ark
{
    Function::Function(Lang::Node::ProcType proc) :
        m_procedure(proc)
    {}
    
    Function::~Function()
    {}
}
