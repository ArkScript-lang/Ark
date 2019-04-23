#include <Ark/Parser/Program.hpp>

namespace Ark
{
    namespace Parser
    {
        Program::Program()
        {}

        Program::~Program()
        {}

        void Program::feed(const std::string& code)
        {
            m_parser.feed(code);
        }

        void Program::execute(const Node& args)
        {
            m_global_env["_args"] = args;
            std::cout << _execute(m_parser.front(), &m_global_env) << std::endl;
        }

        Node Program::_execute(Node x, Environment* env)
        {
            if (x.nodeType() == NodeType::Symbol)
            {
                std::string name = x.getStringVal();
                return env->find(name)[name];
            }
            if (x.nodeType() == NodeType::Number)
                return x;
            if (x.list().empty())
                return nil;
            if (x.list()[0].nodeType() == NodeType::Symbol)
            {
                std::string n = x.list()[0].getStringVal();

                if (n == "quote")
                    return x.list()[1];
                if (n == "if")
                    return _execute(_execute(x.list()[1], env) == falseSym ? x.list()[3] : x.list()[2], env);
                if (n == "set")
                {
                    std::string name = x.list()[1].getStringVal();
                    return env->find(name)[name] = _execute(x.list()[2], env);
                }
                if (n == "def")
                    return (*env)[x.list()[1].getStringVal()] = _execute(x.list()[2], env);
                // fun, begin, while
            }

            // function execution
        }

        std::ostream& operator<<(std::ostream& os, const Program& P)
        {
            os << P.m_parser << std::endl;
            return os;
        }
    }
}
