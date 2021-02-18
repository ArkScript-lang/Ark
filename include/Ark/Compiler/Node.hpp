/**
 * @file Node.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief AST node used by the parser, optimizer and compiler
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef ARK_COMPILER_NODE_HPP
#define ARK_COMPILER_NODE_HPP

#include <variant.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <Ark/Exceptions.hpp>

namespace Ark::internal
{
    /// The different node types available
    enum class NodeType
    {
        Symbol,
        Capture,
        GetField,
        Keyword,
        String,
        Number,
        List,
        Closure,
        Macro,
        Spread
    };

    /// The different keywords available
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

    /**
     * @brief A node of an Abstract Syntax Tree for ArkScript
     * 
     */
    class Node
    {
    public:
        using Iterator = std::vector<Node>::const_iterator;
        using Map      = std::unordered_map<std::string, Node>;
        using Value    = mpark::variant<double, std::string, Keyword>;

        /**
         * @brief Construct a new Node object
         * 
         * @param value 
         */
        explicit Node(int value) noexcept;

        /**
         * @brief Construct a new Node object
         * 
         * @param value 
         */
        explicit Node(double value) noexcept;

        /**
         * @brief Construct a new Node object
         * 
         * @param value 
         */
        explicit Node(const std::string& value) noexcept;

        /**
         * @brief Construct a new Node object
         * 
         * @param value 
         */
        explicit Node(Keyword value) noexcept;

        /**
         * @brief Construct a new Node object, does not set the value
         * 
         * @param type 
         */
        explicit Node(NodeType type=NodeType::Symbol) noexcept;

        /**
         * @brief Construct a new Node object
         * 
         * @param other 
         */
        Node(const Node& other) noexcept;

        /**
         * @brief Return the string held by the value (if the node type allows it)
         * 
         * @return const std::string& 
         */
        const std::string& string() const noexcept;

        /**
         * @brief Return the number held by the value (if the node type allows it)
         * 
         * @return double 
         */
        double number() const noexcept;

        /**
         * @brief Return the keyword held by the value (if the node type allows it)
         * 
         * @return Keyword 
         */
        Keyword keyword() const noexcept;

        /**
         * @brief Every node has a list as well as a value so we can push_back on all node no matter their type
         * 
         * @param node a sub-node to push on the list held by the current node
         */
        void push_back(const Node& node) noexcept;

        /**
         * @brief Return the list of sub-nodes held by the node
         * 
         * @return std::vector<Node>& 
         */
        std::vector<Node>& list() noexcept;

        /**
         * @brief Return the list of sub-nodes held by the node
         * 
         * @return const std::vector<Node>& 
         */
        const std::vector<Node>& const_list() const noexcept;

        /**
         * @brief Return the node type
         * 
         * @return NodeType 
         */
        NodeType nodeType() const noexcept;

        /**
         * @brief Set the Node Type object
         * 
         * @param type 
         */
        void setNodeType(NodeType type) noexcept;

        /**
         * @brief Set the String object
         * 
         * @param value 
         */
        void setString(const std::string& value) noexcept;

        /**
         * @brief Set the Number object
         * 
         * @param value 
         */
        void setNumber(double value) noexcept;

        /**
         * @brief Set the Keyword object
         * 
         * @param kw 
         */
        void setKeyword(Keyword kw) noexcept;

        /**
         * @brief Set the Position of the node in the text
         * 
         * @param line 
         * @param col 
         */
        void setPos(std::size_t line, std::size_t col) noexcept;

        /**
         * @brief Set the original Filename where the node was
         * 
         * @param filename 
         */
        void setFilename(const std::string& filename) noexcept;

        /**
         * @brief Get the line at which this node was created
         * 
         * @return std::size_t 
         */
        std::size_t line() const noexcept;

        /**
         * @brief Get the column at which this node was created
         * 
         * @return std::size_t 
         */
        std::size_t col() const noexcept;

        /**
         * @brief Return the filename in which this node was created
         * 
         * @return const std::string& 
         */
        const std::string& filename() const noexcept;

        friend std::ostream& operator<<(std::ostream& os, const Node& N) noexcept;
        friend inline bool operator==(const Node& A, const Node& B);

    private:
        NodeType m_type;
        Value m_value;
        std::vector<Node> m_list;
        // position of the node in the original code, useful when it comes to parser errors
        std::size_t m_line = 0, m_col = 0;
        std::string m_filename = "";
    };

    #include "Node.inl"

    using Nodes = std::vector<Node>;

    std::ostream& operator<<(std::ostream& os, const Nodes& N) noexcept;
}

#endif
