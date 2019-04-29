#ifndef ark_function
#define ark_function

#include <Ark/Lang/Node.hpp>

#include <Ark/Lang/Program.hpp>
#include <Ark/Lang/Environment.hpp>

namespace Ark
{
    class Function
    {
    public:
        Function(Lang::Program* prog, Lang::Node function);
        
        template <typename... Args>
        Lang::Node operator()(const Args&... args)
        {
            Lang::Nodes exps { args... };
            return m_program->_execute(m_procedure.list()[2], new Lang::Environment(m_procedure.list()[1].list(), exps, m_procedure.getEnv()));
        }
    
    private:
        Lang::Node m_procedure;
        Lang::Program* m_program;
    };
}

#endif  // ark_function
