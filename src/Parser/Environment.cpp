#include <Ark/Parser/Environment.hpp>

#include <Ark/Log.hpp>

namespace Ark
{
    namespace Parser
    {
        Environment::Environment(Environment* outer) :
            m_outer(outer)
        {}

        Environment::Environment(Nodes& params, Nodes& args, Environment* outer) :
            m_outer(outer)
        {
            Node::Iterator it = args.begin();
            for (Node::Iterator p=params.begin(); p != params.end(); ++p)
                m_env[p->getStringVal()] = *it++;
        }

        Environment::Map& Environment::find(const std::string& var)
        {
            if (m_env.find(var) != m_env.end())
                return m_env;
            
            if (m_outer != nullptr)
                return m_outer->find(var);
            
            Ark::Log::error("[Environment] Unbound symbol: " + var);
            exit(1);
        }

        Node& Environment::operator[](const std::string& var)
        {
            return m_env[var];
        }
    }
}