#ifndef ark_node
#define ark_node

#include <variant>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

namespace Ark
{
    namespace Parser
    {
        enum class NodeType
        {
            Symbol,
            Number,
            List,
            Proc,
            Lambda
        };

        enum class ValueType
        {
            Int,
            Float,
            String
        };

        class Environment;

        class Node
        {
        public:
            using ProcType = Node(*)(const std::vector<Node>&);
            using Iterator = std::vector<Node>::iterator;
            using Map = std::unordered_map<std::string, Node>;
            using Value = std::variant<int, float, std::string>;

            Node(NodeType type=NodeType::Symbol);
            template <typename T> Node(NodeType type, const T& value);
            Node(Node::ProcType proc);
            ~Node();

            void addEnv(Environment* env);
            const std::string& getStringVal();
            void push_back(const Node& node);
            NodeType nodeType();
            ValueType valueType();
            std::vector<Node>& list();

            friend std::ostream& operator<<(std::ostream& os, const Node& N);
        
        private:
            NodeType m_type;
            Value m_value;
            ValueType m_valuetype;
            std::vector<Node> m_list;
            Node::ProcType m_procedure;
            Environment* m_env;
        };

        inline bool operator==(const Node& A, const Node& B)
        {
            if (A.m_type == B.m_type && A.m_type != NodeType::List && A.m_type != NodeType::Proc && A.m_type != NodeType::Lambda)
                return A.m_valuetype == B.m_valuetype;
            return false;  //! not comparing proc/list/lambda
        }

        const Node nil(NodeType::Symbol, "nil");
        const Node falseSym(NodeType::Symbol, "#f");
        const Node trueSym(NodeType::Symbol, "#t");

        using Nodes = std::vector<Node>;
    }
}

#endif  // ark_node
