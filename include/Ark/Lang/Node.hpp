#ifndef ark_node
#define ark_node

#include <variant>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <Ark/BigNum.hpp>
#include <Ark/Exceptions.hpp>

namespace Ark
{
    namespace Lang
    {
        enum class NodeType
        {
            Symbol,
            Keyword,
            String,
            Number,
            List,
            Proc,
            Closure
        };

        enum class Keyword
        {
            Fun,
            Let,
            Set,
            If,
            While,
            Begin,
            Import
        };

        class Environment;

        class Node
        {
        public:
            using ProcType = Node(*)(const std::vector<Node>&);
            using Iterator = std::vector<Node>::const_iterator;
            using Map = std::unordered_map<std::string, Node>;
            using Value = std::variant<BigNum, std::string>;

            Node(int value);
            Node(const BigNum& value);
            Node(const std::string& value);
            Node(NodeType type=NodeType::Symbol);
            Node(NodeType type, int value);
            Node(NodeType type, const BigNum& value);
            Node(NodeType type, const std::string& value);
            Node(NodeType type, Keyword value);
            Node(Node::ProcType proc);

            void addEnv(Environment* env);
            Environment* getEnv();

            const std::string& getStringVal() const;
            const BigNum getIntVal() const;
            const ProcType getProcVal() const;
            void push_back(const Node& node);

            const NodeType& nodeType() const;
            void setNodeType(NodeType type);
            const Keyword keyword() const;
            void setKeyword(Keyword kw);

            std::vector<Node>& list();
            const std::vector<Node>& const_list() const;

            Node call(const std::vector<Node>& args);

            void setPos(std::size_t line, std::size_t col);
            std::size_t line() const;
            std::size_t col() const;

            friend std::ostream& operator<<(std::ostream& os, const Node& N);
            friend inline bool operator==(const Node& A, const Node& B);

            inline std::string typeToString() const
            {
                // must have the same order as the enum class NodeType L17
                static const std::vector<std::string> nodetype_str = {
                    "Symbol", "Keyword", "String", "Number", "List", "Procedure", "Closure"
                };

                if (m_type == NodeType::Symbol)
                {
                    if (getStringVal() == "nil")
                        return "Nil";
                    return "Bool";
                }
                
                return nodetype_str[static_cast<int>(m_type)];
            }

        private:
            NodeType m_type;
            Value m_value;

            Keyword m_keyword;

            std::vector<Node> m_list;

            Node::ProcType m_procedure;
            Environment* m_env;

            std::size_t m_line, m_col;
        };

        inline bool operator==(const Node& A, const Node& B)
        {
            if (A.m_type != B.m_type)  // should have the same types
                return false;

            if (A.m_type != NodeType::List &&
                A.m_type != NodeType::Proc &&
                A.m_type != NodeType::Closure)
                return A.m_value == B.m_value;
            
            if (A.m_type == NodeType::List)
                throw Ark::TypeError("Can not compare lists");
            
            if (A.m_type == NodeType::Proc)
                return A.m_procedure == B.m_procedure;
            
            // any other type => false (here, Closure)
            return false;
        }

        extern const Node nil;
        extern const Node falseSym;
        extern const Node trueSym;

        using Nodes = std::vector<Node>;
    }
}

#endif  // ark_node
