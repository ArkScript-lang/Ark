#include <Ark/Lang/Node.hpp>

#include <Ark/Lang/Environment.hpp>

namespace Ark
{
    namespace Lang
    {
        Node::Node(NodeType type) :
            m_type(type), m_env(nullptr)
        {}

        template <> Node::Node<int>(NodeType type, const int& value) :
            m_type(type),
            m_value(value),
            m_valuetype(ValueType::Int),
            m_env(nullptr)
        {}

        template <> Node::Node<float>(NodeType type, const float& value) :
            m_type(type),
            m_value(value),
            m_valuetype(ValueType::Float),
            m_env(nullptr)
        {}

        template <> Node::Node<std::string>(NodeType type, const std::string& value) :
            m_type(type),
            m_value(value),
            m_valuetype(ValueType::String),
            m_env(nullptr)
        {}

        Node::Node(Node::ProcType proc) :
            m_procedure(proc),
            m_type(NodeType::Proc),
            m_env(nullptr)
        {}

        Node::~Node()
        {}

        void Node::addEnv(Environment* env)
        {
            m_env = env;
        }

        Environment* Node::getEnv()
        {
            return m_env;
        }

        const std::string& Node::getStringVal() const
        {
            return std::get<std::string>(m_value);
        }

        const int Node::getIntVal() const
        {
            return std::get<int>(m_value);
        }

        const float Node::getFloatVal() const
        {
            return std::get<float>(m_value);
        }

        void Node::push_back(const Node& node)
        {
            m_list.push_back(node);
        }

        const NodeType& Node::nodeType() const
        {
            return m_type;
        }

        void Node::setNodeType(NodeType type)
        {
            m_type = type;
        }

        const ValueType Node::valueType() const
        {
            return m_valuetype;
        }

        std::vector<Node>& Node::list()
        {
            return m_list;
        }

        const std::vector<Node>& Node::const_list() const
        {
            return m_list;
        }

        Node Node::call(const std::vector<Node>& args)
        {
            return m_procedure(args);
        }

        std::ostream& operator<<(std::ostream& os, const Node& N)
        {
            switch(N.m_type)
            {
            case NodeType::Symbol:
                os << std::get<std::string>(N.m_value);
                break;

            case NodeType::Number:
                if (N.m_valuetype == ValueType::Int)
                    os << std::get<int>(N.m_value);
                else  // assuming it's a float
                    os << std::get<float>(N.m_value);
                break;

            case NodeType::List:
            {
                int i = 0;
                os << "( ";
                for (auto& t: N.m_list)
                    os << t << " ";
                os << ")";
                break;
            }

            case NodeType::Proc:
                os << "Procedure";
                break;

            case NodeType::Lambda:
                os << "Lambda";
                break;

            default:
                os << "~\\._./~";
                break;
            }
            return os;
        }

        extern const Node nil = Node(NodeType::Symbol, std::string("nil"));
        extern const Node falseSym = Node(NodeType::Symbol, std::string("false"));
        extern const Node trueSym = Node(NodeType::Symbol, std::string("true"));
    }
}
