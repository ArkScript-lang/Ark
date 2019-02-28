#ifndef ark_function
#define ark_function

#include <Ark/Lang/Node.hpp>

namespace Ark
{
    class Function
    {
    public:
        Function(Lang::Node::ProcType proc);
        ~Function();
        
        template <typename... Args>
        Lang::Node operator()(const Args&... args)
        {
            return m_procedure(Lang::Nodes { args... });
        }
    
    private:
        Lang::Node::ProcType m_procedure;
    };
}

#endif  // ark_function
