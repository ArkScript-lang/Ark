#ifndef ark_node
#define ark_node

#include <variant>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <huge_number.hpp>

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
            Lambda
        };

        enum class Keyword
        {
            Fun,
            Def,
            Set,
            If,
            While,
            Begin,
            HasType
        };

        class Environment;

        class Node
        {
        public:
            using ProcType = Node(*)(const std::vector<Node>&);
            using Iterator = std::vector<Node>::const_iterator;
            using Map = std::unordered_map<std::string, Node>;
            using Value = std::variant<dozerg::HugeNumber, std::string>;

            template <typename T> Node(const T& value);
            Node(NodeType type=NodeType::Symbol);
            template <typename T> Node(NodeType type, const T& value);
            Node(Node::ProcType proc);
            ~Node();

            void addEnv(Environment* env);
            Environment* getEnv();

            const std::string& getStringVal() const;
            const dozerg::HugeNumber getIntVal() const;
            const ProcType getProcVal() const;
            void push_back(const Node& node);

            const NodeType& nodeType() const;
            void setNodeType(NodeType type);
            const Keyword keyword() const;

            std::vector<Node>& list();
            const std::vector<Node>& const_list() const;

            Node call(const std::vector<Node>& args);

            void setPos(std::size_t line, std::size_t col);
            std::size_t line() const;
            std::size_t col() const;

            friend std::ostream& operator<<(std::ostream& os, const Node& N);
            friend inline bool operator==(const Node& A, const Node& B);

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
            if (A.m_type == B.m_type && A.m_type != NodeType::List && A.m_type != NodeType::Proc && A.m_type != NodeType::Lambda)
                return A.m_value == B.m_value;
            return false;  //! not comparing proc/list/lambda
        }

        extern const Node nil;
        extern const Node falseSym;
        extern const Node trueSym;

        using Nodes = std::vector<Node>;
    }
}

#endif  // ark_node
