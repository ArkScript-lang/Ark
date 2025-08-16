/**
 * @file Node.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief AST node used by the parser, optimizer and compiler
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2025
 *
 */

#ifndef COMPILER_AST_NODE_HPP
#define COMPILER_AST_NODE_HPP

#include <variant>
#include <ostream>
#include <string>
#include <vector>
#include <optional>

#include <Ark/Compiler/AST/Namespace.hpp>
#include <Ark/Compiler/Common.hpp>
#include <Ark/Utils/Platform.hpp>

namespace Ark::internal
{
    // todo: remove and use codeerrorcontext -> contextposition
    struct NodePos
    {
        std::size_t start_line = 0;  // todo: do not default init to track the last nodes without a position inside the parser (?)
        std::size_t start_col = 0;
        std::size_t end_line = 0;
        std::size_t end_col = 0;
    };

    /**
     * @brief A node of an Abstract Syntax Tree for ArkScript
     *
     */
    class ARK_API Node
    {
    public:
        using Value = std::variant<double, std::string, Keyword, std::vector<Node>, Namespace>;

        Node() = default;

        Node(NodeType node_type, const std::string& value);

        explicit Node(NodeType node_type);
        explicit Node(double value);
        explicit Node(long value);
        explicit Node(Keyword value);
        explicit Node(const Namespace& namespace_);

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
         * @brief Return the namespace held by the value (if the node type allows it)
         *
         * @return Namespace&
         */
        [[nodiscard]] Namespace& arkNamespace() noexcept;

        /**
         * @brief Return the namespace held by the value (if the node type allows it)
         *
         * @return const Namespace&
         */
        [[nodiscard]] const Namespace& constArkNamespace() const noexcept;

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
         * @brief Check if the node is a string like node
         * @return true if the node is either a symbol, a string or a spread
         * @return false
         */
        [[nodiscard]] bool isStringLike() const noexcept;

        /**
         * @brief Check if the node is a function
         * @return true if the node is a function declaration
         * @return false
         */
        [[nodiscard]] bool isFunction() const noexcept;

        /**
         * @brief Get the unqualified name, if it has been set
         *
         * @return const std::optional<std::string>&
         */
        [[nodiscard]] const std::optional<std::string>& getUnqualifiedName() const noexcept;

        /**
         * @brief Copy a node to the current one, while keeping the filename and position in the file
         *
         * @param source node to copy type and value from
         */
        void updateValueAndType(const Node& source) noexcept;

        /**
         * @brief Set the Node Type object
         *
         * @param type
         */
        void setNodeType(NodeType type) noexcept;

        /**
         * @brief Set the unqualified name (used by Capture nodes)
         *
         * @param name
         */
        void setUnqualifiedName(const std::string& name) noexcept;

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

        /**
         * @brief Set the comment_after field with the nearest comment after this node
         * @param comment
         * @return Node& reference to this node after updating it
         */
        Node& attachCommentAfter(const std::string& comment);

        /**
         * @brief Set the m_alt_syntax flag of the node
         * @param toggle
         */
        void setAltSyntax(bool toggle);

        /**
         * @brief Set the m_is_anonymous_function flag on the node
         * @param anonymous true to mark the node as an anonymous function
         */
        void setFunctionKind(bool anonymous);

        /**
         * @brief Check if a node is an anonymous function
         * @return true if the node is of an anonymous function
         * @return false
         */
        [[nodiscard]] bool isAnonymousFunction() const noexcept;

        /**
         * @brief Check if a node is alt syntax
         * @return bool
         */
        [[nodiscard]] bool isAltSyntax() const;

        [[nodiscard]] std::size_t line() const noexcept;
        [[nodiscard]] std::size_t col() const noexcept;

        /**
         * @brief Get the position of the node (start and end)
         *
         * @return const NodePos
         */
        [[nodiscard]] NodePos position() const noexcept;

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

        /**
         * @brief Return the comment attached after this node, if any
         * @return const std::string&
         */
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

    private:
        NodeType m_type { NodeType::Unused };
        Value m_value;
        std::optional<std::string> m_unqualified_name { std::nullopt };  ///< Used by Capture nodes, to have the FQN in the value, and the captured name here
        // position of the node in the original code, useful when it comes to parser errors
        NodePos m_pos;
        std::string m_filename;
        std::string m_comment;
        std::string m_after_comment;          ///< Comment after node
        bool m_alt_syntax = false;            ///< Used to tell if a node uses the alternative syntax (if available), eg (begin) / {}, (list) / []
        bool m_is_anonymous_function = true;  ///< Function nodes are marked as anonymous/non-anonymous by the ASTLowerer, to enable some optimisations
    };

    const Node& getTrueNode();
    const Node& getFalseNode();
    const Node& getNilNode();
    const Node& getListNode();

    /**
     *
     * @param node
     * @return std::string a string corresponding to the node type
     */
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
