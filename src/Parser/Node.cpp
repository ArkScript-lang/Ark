#include <Ark/Parser/Node.hpp>

namespace Ark
{
    namespace Parser
    {
        Node::Node()
        {}

        Node::~Node()
        {}

        std::ostream& operator<<(std::ostream& os, const Node& N)
        {
            os << "\t";
            switch(N.m_type)
            {
            case NodeType::Def:
                os << std::get<Definition>(N.m_value);
                break;
            
            case NodeType::Set:
                os << std::get<Set>(N.m_value);
                break;
            
            case NodeType::Fun:
                os << std::get<Function>(N.m_value);
                break;
            
            case NodeType::If:
                os << std::get<IfCond>(N.m_value);
                break;
            
            case NodeType::While:
                os << std::get<WhileLoop>(N.m_value);
                break;
            
            case NodeType::Value:
                os << std::get<Value>(N.m_value);
                break;
            
            case NodeType::Block:
                os << std::get<Block>(N.m_value);
                break;
            
            default:
                os << "Did I forget to add a node type?" << std::endl;
                break;
            }
            return os;
        }
    }
}