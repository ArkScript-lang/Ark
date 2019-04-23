#include <Ark/Lang/Program.hpp>

#include <Ark/Log.hpp>
#include <Ark/Lang/Lib.hpp>
#include <Ark/Function.hpp>

namespace Ark
{
    namespace Lang
    {
        Program::Program(bool debug) :
            m_debug(debug),
            m_parser(debug)
        {}

        Program::~Program()
        {}

        void Program::feed(const std::string& code)
        {
            m_parser.feed(code);
            
            if (!m_parser.check())
            {
                Ark::logger.error("[Program] Program has errors");
                exit(1);
            }

            if (m_debug)
                Ark::logger.info("(Program) Program parsed and checked without errors");
        }

        void Program::execute(const Nodes& args)
        {
            Node argslist(NodeType::List);
            for (auto& node: args)
                argslist.push_back(node);

            m_global_env["_args"] = argslist;
            registerLib(m_global_env);

            _execute(m_parser.ast(), &m_global_env);
        }

        void Program::loadFunction(const std::string& name, Node::ProcType function)
        {
            m_global_env[name] = Node(function);
        }
        
        template <> BigNum Program::get<BigNum>(const std::string& key)
        {
            Node& n = m_global_env.find(key)[key];
            if (n.nodeType() == NodeType::Number)
                return n.getIntVal();
            Ark::logger.error("[Program] '" + key + "' isn't an integer");
            exit(1);
        }
        
        template <> std::string Program::get<std::string>(const std::string& key)
        {
            Node& n = m_global_env.find(key)[key];
            if (n.nodeType() == NodeType::String)
                return n.getStringVal();
            Ark::logger.error("[Program] '" + key + "' isn't a string");
            exit(1);
        }
        
        template <> Function Program::get<Function>(const std::string& key)
        {
            Node& n = m_global_env.find(key)[key];
            if (n.nodeType() == NodeType::Lambda)
                return Function(this, n);
            Ark::logger.error("[Program] '" + key + "' isn't a function");
            exit(1);
        }

        Node Program::_execute(Node x, Environment* env)
        {
            if (x.nodeType() == NodeType::Symbol)
            {
                std::string name = x.getStringVal();
                return env->find(name)[name];
            }
            if (x.nodeType() == NodeType::String || x.nodeType() == NodeType::Number)
                return x;
            if (x.list().empty())
                return nil;
            if (x.list()[0].nodeType() == NodeType::Keyword)
            {
                Keyword n = x.list()[0].keyword();

                if (n == Keyword::HasType)
                {
                    // skip has-type rules in interpreter
                    return nil;
                }

                if (n == Keyword::If)
                    return _execute((_execute(x.list()[1], env) == falseSym) ? x.list()[3] : x.list()[2], env);
                if (n == Keyword::Set)
                {
                    std::string name = x.list()[1].getStringVal();
                    return env->find(name)[name] = _execute(x.list()[2], env);
                }
                if (n == Keyword::Def)
                    return (*env)[x.list()[1].getStringVal()] = _execute(x.list()[2], env);
                if (n == Keyword::Fun)
                {
                    x.setNodeType(NodeType::Lambda);
                    x.addEnv(env);
                    return x;
                }
                if (n == Keyword::Begin)
                {
                    for (std::size_t i=1; i < x.list().size() - 1; ++i)
                        _execute(x.list()[i], env);
                    return _execute(x.list()[x.list().size() - 1], env);
                }
                if (n == Keyword::While)
                {
                    while (_execute(x.list()[1], env) == trueSym)
                        _execute(x.list()[2], env);
                    return nil;
                }
            }

            Node proc(_execute(x.list()[0], env));
            Nodes exps;
            for (Node::Iterator exp=x.list().begin() + 1; exp != x.list().end(); ++exp)
                exps.push_back(_execute(*exp, env));

            if (proc.nodeType() == NodeType::Lambda)
                return _execute(proc.list()[2], new Environment(proc.list()[1].list(), exps, proc.getEnv()));
            else if (proc.nodeType() == NodeType::Proc)
                return proc.call(exps);
            else
            {
                Ark::logger.error("[Program] not a function");
                exit(1);
            }
        }

        std::ostream& operator<<(std::ostream& os, const Program& P)
        {
            os << "Environment" << std::endl;
            if (P.m_global_env.empty())
                os << "=> empty";
            else
                os << P.m_global_env << std::endl;

            return os;
        }
    }
}
