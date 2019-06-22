#ifndef ark_node
#define ark_node

#include <variant>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <Ark/Exceptions.hpp>

namespace Ark
{
    namespace internal
    {
        enum class NodeType
        {
            Symbol,
            Keyword,
            String,
            Number,
            List,
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
            Import,
            Quote
        };

        class Node
        {
        public:
            using Iterator = std::vector<Node>::const_iterator;
            using Map = std::unordered_map<std::string, Node>;
            using Value = std::variant<double, std::string, Keyword>;

            Node(int value);
            Node(double value);
            Node(const std::string& value);
            Node(Keyword value);
            Node(NodeType type=NodeType::Symbol);

            const std::string& string() const;
            double number() const;
            Keyword keyword() const;

            void push_back(const Node& node);
            std::vector<Node>& list();
            const std::vector<Node>& const_list() const;

            NodeType nodeType() const;
            void setNodeType(NodeType type);
            void setString(const std::string& value);
            void setNumber(double value);
            void setKeyword(Keyword kw);

            void setPos(std::size_t line, std::size_t col);
            std::size_t line() const;
            std::size_t col() const;

            friend std::ostream& operator<<(std::ostream& os, const Node& N);
            friend inline bool operator==(const Node& A, const Node& B);

            inline std::string typeToString() const
            {
                // must have the same order as the enum class NodeType L17
                static const std::vector<std::string> nodetype_str = {
                    "Symbol", "Keyword", "String", "Number", "List", "Closure"
                };

                if (m_type == NodeType::Symbol)
                {
                    if (string() == "nil")
                        return "Nil";
                    return "Bool";
                }
                
                return nodetype_str[static_cast<int>(m_type)];
            }

        private:
            NodeType m_type;
            Value m_value;

            std::vector<Node> m_list;

            std::size_t m_line, m_col;
        };

        inline bool operator==(const Node& A, const Node& B)
        {
            if (A.m_type != B.m_type)  // should have the same types
                return false;

            if (A.m_type != NodeType::List &&
                A.m_type != NodeType::Closure)
                return A.m_value == B.m_value;
            
            if (A.m_type == NodeType::List)
                throw Ark::TypeError("Can not compare lists");
            
            // any other type => false (here, Closure)
            return false;
        }

        using Nodes = std::vector<Node>;

        std::ostream& operator<<(std::ostream& os, const Nodes& N);
    }
}

#endif  // ark_node
