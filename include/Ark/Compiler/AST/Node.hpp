/**
 * @file Node.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief AST node used by the parser, optimizer and compiler
 * @version 0.4
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2021
 *
 */

#ifndef COMPILER_AST_NODE_HPP
#define COMPILER_AST_NODE_HPP

#include <variant>
#include <ostream>
#include <string>
#include <vector>

#include <Ark/Compiler/Common.hpp>
#include <Ark/Platform.hpp>

namespace Ark::internal
{
    /**
     * @brief A node of an Abstract Syntax Tree for ArkScript
     *
     */
    class ARK_API Node
    {
    public:
        using Value = std::variant<double, std::string, Keyword, std::vector<Node>>;

        Node() = default;

        Node(NodeType node_type, const std::string& value);

        explicit Node(NodeType node_type);
        explicit Node(double value);
        explicit Node(long value);
        explicit Node(int value);
        explicit Node(Keyword value);
        explicit Node(const std::vector<Node>& nodes);

        /**
         * @brief Return the string held by the value (if the node type allows it)
         *
         * @return const std::string&
         */
        [[nodiscard]] const std::string& string() const noexcept;

        /**
         * @brief Return the number held by the value (if the node type allows it)
         *
         * @return double
         */
        [[nodiscard]] double number() const noexcept;

        /**
         * @brief Return the keyword held by the value (if the node type allows it)
         *
         * @return Keyword
         */
        [[nodiscard]] Keyword keyword() const noexcept;

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
        [[nodiscard]] const std::vector<Node>& constList() const noexcept;

        /**
         * @brief Return the node type
         *
         * @return NodeType
         */
        [[nodiscard]] NodeType nodeType() const noexcept;

        /**
         * @brief Check if the node is a list like node
         * @return true if the node is either a list or a macro
         * @return false
         */
        [[nodiscard]] bool isListLike() const noexcept;

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
         * @brief Set the comment field with the nearest comment before this node
         * @param comment
         * @return Node& reference to this node after updating it
         */
        Node& attachNearestCommentBefore(const std::string& comment);

        Node& attachCommentAfter(const std::string& comment);

        /**
         * @brief Get the line at which this node was created
         *
         * @return std::size_t
         */
        [[nodiscard]] std::size_t line() const noexcept;

        /**
         * @brief Get the column at which this node was created
         *
         * @return std::size_t
         */
        [[nodiscard]] std::size_t col() const noexcept;

        /**
         * @brief Return the filename in which this node was created
         *
         * @return const std::string&
         */
        [[nodiscard]] const std::string& filename() const noexcept;

        /**
         * @brief Return the comment attached to this node, if any
         * @return const std::string&
         */
        [[nodiscard]] const std::string& comment() const noexcept;

        [[nodiscard]] const std::string& commentAfter() const noexcept;

        /**
         * @brief Compute a representation of the node without any comments or additional sugar, colors, types
         * @return String representation of the node
         */
        [[nodiscard]] std::string repr() const noexcept;

        /**
         * @brief Print a node to an output stream with added type annotations
         * @param os
         * @return
         */
        [[nodiscard]] std::ostream& debugPrint(std::ostream& os) const noexcept;

        friend bool operator==(const Node& A, const Node& B);
        friend bool operator<(const Node& A, const Node& B);
        friend bool operator!(const Node& A);

    private:
        NodeType m_type;
        Value m_value;
        // position of the node in the original code, useful when it comes to parser errors
        std::size_t m_line = 0, m_col = 0;
        std::string m_filename;
        std::string m_comment;
        std::string m_after_comment;  ///< Comment after node
    };

    const Node& getTrueNode();
    const Node& getFalseNode();
    const Node& getNilNode();
    const Node& getListNode();

    inline std::string typeToString(const Node& node) noexcept
    {
        if (node.nodeType() == NodeType::Symbol)
        {
            if (node.string() == "nil")
                return "Nil";
            if (node.string() == "true" || node.string() == "false")
                return "Bool";
        }

        const auto c = static_cast<std::size_t>(node.nodeType());
        return (c < nodeTypes.size()) ? std::string(nodeTypes[c]) : "???";
    }
}

#endif
