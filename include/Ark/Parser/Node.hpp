#ifndef ark_node
#define ark_node

#include <variant>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <Ark/Exceptions.hpp>

namespace Ark::internal
{
    enum class NodeType
    {
        Symbol,
        Capture,
        GetField,
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
        Mut,
        Set,
        If,
        While,
        Begin,
        Import,
        Quote,
        Del
    };

    class Node
    {
    public:
        using Iterator = std::vector<Node>::const_iterator;
        using Map      = std::unordered_map<std::string, Node>;
        using Value    = std::variant<double, std::string, Keyword>;

        // Numbers
        Node(int value);
        Node(double value);
        // Strings
        Node(const std::string& value);
        // Keywords
        Node(Keyword value);
        // Default constructor, not setting the value
        Node(NodeType type=NodeType::Symbol);

        // getters
        const std::string& string() const;
        double number() const;
        Keyword keyword() const;

        // every node has a list as well as a value so we can push_back on all node no matter their type
        void push_back(const Node& node);
        std::vector<Node>& list();
        const std::vector<Node>& const_list() const;

        NodeType nodeType() const;

        // setters
        void setNodeType(NodeType type);
        void setString(const std::string& value);
        void setNumber(double value);
        void setKeyword(Keyword kw);

        void setPos(std::size_t line, std::size_t col);
        std::size_t line() const;
        std::size_t col() const;

        friend std::ostream& operator<<(std::ostream& os, const Node& N);
        friend inline bool operator==(const Node& A, const Node& B);

    private:
        NodeType m_type;
        Value m_value;
        std::vector<Node> m_list;
        // position of the node in the original code, useful when it comes to parser errors
        std::size_t m_line, m_col;
    };

    #include "Node.inl"

    using Nodes = std::vector<Node>;

    std::ostream& operator<<(std::ostream& os, const Nodes& N);
}

#endif  // ark_node
