#include <Ark/Lang/Environment.hpp>

#include <Ark/Log.hpp>

namespace Ark
{
    namespace Lang
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

        std::ostream& operator<<(std::ostream& os, const Environment& E)
        {
            for (auto& kv: E.m_env)
                os << kv.first << ":" << kv.second << std::endl;

            if (E.m_outer != nullptr)
            {
                os << std::endl;
                os << "Outer:" << std::endl;
                os << (*E.m_outer) << std::endl;
            }

            return os;
        }
    }
}
