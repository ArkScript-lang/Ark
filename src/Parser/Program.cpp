#include <Ark/Parser/Program.hpp>

namespace Ark
{
    namespace Parser
    {
        Program::Program()
        {}

        Program::~Program()
        {}

        void Program::add(const Node& node)
        {
            m_program.push_back(node);
        }

        std::ostream& operator<<(std::ostream& os, const Program& P)
        {
            for (auto& node: P.m_program)
            {
                os << node << std::endl;
            }
            return os;
        }
    }
}