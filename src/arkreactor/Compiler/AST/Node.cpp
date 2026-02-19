#include <Ark/Compiler/Common.hpp>
#include <Ark/Compiler/AST/Node.hpp>

#include <Ark/Error/Exceptions.hpp>

#include <cassert>
#include <fmt/core.h>

namespace Ark::internal
{
    Node::Node(const NodeType node_type, const std::string& value) :
        m_type(node_type), m_value(value), m_pos()
    {}

    Node::Node(const NodeType node_type) :
        m_type(node_type), m_pos()
    {
        if (m_type == NodeType::List || m_type == NodeType::Macro || m_type == NodeType::Field)
            m_value = std::vector<Node>();
    }

    Node::Node(double value) :
        m_type(NodeType::Number), m_value(value), m_pos()
    {}

    Node::Node(const long value) :
        m_type(NodeType::Number), m_value(static_cast<double>(value)), m_pos()
    {}

    Node::Node(Keyword value) :
        m_type(NodeType::Keyword), m_value(value), m_pos()
    {}

    Node::Node(const Namespace& namespace_) :
        m_type(NodeType::Namespace), m_value(namespace_), m_pos()
    {}

    const std::string& Node::string() const noexcept
    {
        return std::get<std::string>(m_value);
    }

    double Node::number() const noexcept
    {
        return std::get<double>(m_value);
    }

    Keyword Node::keyword() const noexcept
    {
        return std::get<Keyword>(m_value);
    }

    Namespace& Node::arkNamespace() noexcept
    {
        return std::get<Namespace>(m_value);
    }

    const Namespace& Node::constArkNamespace() const noexcept
    {
        return std::get<Namespace>(m_value);
    }

    void Node::push_back(const Node& node) noexcept
    {
        list().push_back(node);
    }

    std::vector<Node>& Node::list() noexcept
    {
        return std::get<std::vector<Node>>(m_value);
    }

    const std::vector<Node>& Node::constList() const noexcept
    {
        return std::get<std::vector<Node>>(m_value);
    }

    NodeType Node::nodeType() const noexcept
    {
        return m_type;
    }

    bool Node::isListLike() const noexcept
    {
        return m_type == NodeType::List || m_type == NodeType::Macro;
    }

    bool Node::isFunction() const noexcept
    {
        return m_type == NodeType::List &&
            !constList().empty() &&
            constList()[0].nodeType() == NodeType::Keyword &&
            constList()[0].keyword() == Keyword::Fun;
    }

    const std::optional<std::string>& Node::getUnqualifiedName() const noexcept
    {
        return m_unqualified_name;
    }

    void Node::updateValueAndType(const Node& source) noexcept
    {
        m_type = source.m_type;
        m_value = source.m_value;
    }

    void Node::setNodeType(const NodeType type) noexcept
    {
        m_type = type;
    }

    void Node::setUnqualifiedName(const std::string& name) noexcept
    {
        m_unqualified_name = name;
    }

    void Node::setString(const std::string& value) noexcept
    {
        m_value = value;
    }

    void Node::setPositionFrom(const Node& source) noexcept
    {
        m_filename = source.m_filename;
        m_pos = source.m_pos;
    }

    Node& Node::attachNearestCommentBefore(const std::string& comment)
    {
        m_comment = comment;
        return *this;
    }

    Node& Node::attachCommentAfter(const std::string& comment)
    {
        if (!m_after_comment.empty())
            m_after_comment += "\n";
        m_after_comment += comment;
        if (!m_after_comment.empty() && m_after_comment.back() == '\n')
            m_after_comment.pop_back();
        return *this;
    }

    void Node::setAltSyntax(const bool toggle)
    {
        m_alt_syntax = toggle;
    }

    void Node::setFunctionKind(const bool anonymous)
    {
        m_is_anonymous_function = anonymous;
    }

    bool Node::isAnonymousFunction() const noexcept
    {
        return m_is_anonymous_function;
    }

    FileSpan Node::position() const noexcept
    {
        return m_pos;
    }

    const std::string& Node::filename() const noexcept
    {
        return m_filename;
    }

    const std::string& Node::comment() const noexcept
    {
        return m_comment;
    }

    const std::string& Node::commentAfter() const noexcept
    {
        return m_after_comment;
    }

    std::string Node::repr() const noexcept
    {
        std::string data;
        switch (m_type)
        {
            case NodeType::Symbol:
                data += string();
                break;

            case NodeType::MutArg:
                data += "(mut " + string() + ")";
                break;

            case NodeType::RefArg:
                data += "(ref " + string() + ")";
                break;

            case NodeType::Capture:
                data += "&" + string();
                break;

            case NodeType::Keyword:
                data += keywords[static_cast<std::size_t>(keyword())];
                break;

            case NodeType::String:
                data += "\"" + string() + "\"";
                break;

            case NodeType::Number:
                data += fmt::format("{}", number());
                break;

            case NodeType::List:
                if (m_alt_syntax)
                {
                    const auto first = constList().front();
                    char open = 0;
                    if (first.nodeType() == NodeType::Keyword && first.keyword() == Keyword::Begin)
                        open = '{';
                    else if (first.nodeType() == NodeType::Symbol && first.string() == "list")
                        open = '[';
                    else
                        assert(false && "Alt syntax nodes can only be begin or list");

                    data += open;
                    for (std::size_t i = 1, end = constList().size(); i < end; ++i)
                    {
                        data += constList()[i].repr();
                        if (i < end - 1)
                            data += " ";
                    }

                    if (open == '{')
                        data += "}";
                    else if (open == '[')
                        data += "]";
                }
                else
                {
                    data += "(";
                    for (std::size_t i = 0, end = constList().size(); i < end; ++i)
                    {
                        data += constList()[i].repr();
                        if (i < end - 1)
                            data += " ";
                    }
                    data += ")";
                }
                break;

            case NodeType::Field:
                for (std::size_t i = 0, end = constList().size(); i < end; ++i)
                {
                    data += constList()[i].repr();
                    if (i < end - 1)
                        data += ".";
                }
                break;

            case NodeType::Macro:
                data += "(macro ";
                for (std::size_t i = 0, end = constList().size(); i < end; ++i)
                {
                    data += constList()[i].repr();
                    if (i < end - 1)
                        data += " ";
                }
                data += ")";
                break;

            case NodeType::Spread:
                data += "..." + string();
                break;

            // namespace node should not have a representation as it is purely internal,
            // and it can't be exploited by macros (unless you try passing an import node
            // to a macro, which should not happen?)
            case NodeType::Namespace:
                data += constArkNamespace().ast->repr();
                break;

            case NodeType::Unused:
                break;
        }
        return data;
    }

    std::ostream& Node::debugPrint(std::ostream& os) const noexcept
    {
        switch (m_type)
        {
            case NodeType::Symbol:
                os << "Symbol:" << string();
                break;

            case NodeType::MutArg:
                os << "MutArg:" << string();
                break;

            case NodeType::RefArg:
                os << "RefArg:" << string();
                break;

            case NodeType::Capture:
                os << "Capture:" << string();
                break;

            case NodeType::Keyword:
                os << "Keyword:";
                switch (keyword())
                {
                    case Keyword::Fun: os << "Fun"; break;
                    case Keyword::Let: os << "Let"; break;
                    case Keyword::Mut: os << "Mut"; break;
                    case Keyword::Set: os << "Set"; break;
                    case Keyword::If: os << "If"; break;
                    case Keyword::While: os << "While"; break;
                    case Keyword::Begin: os << "Begin"; break;
                    case Keyword::Import: os << "Import"; break;
                    case Keyword::Del: os << "Del"; break;
                }
                break;

            case NodeType::String:
                os << "String:" << string();
                break;

            case NodeType::Number:
                os << "Number:" << number();
                break;

            case NodeType::List:
                os << "( ";
                for (const auto& i : constList())
                    i.debugPrint(os) << " ";
                os << ")";
                break;

            case NodeType::Field:
                os << "( Field ";
                for (const auto& i : constList())
                    i.debugPrint(os) << " ";
                os << ")";
                break;

            case NodeType::Macro:
                os << "( Macro ";
                for (const auto& i : constList())
                    i.debugPrint(os) << " ";
                os << ")";
                break;

            case NodeType::Spread:
                os << "Spread:" << string();
                break;

            case NodeType::Namespace:
            {
                const auto details = constArkNamespace();
                os << "( Namespace:" << details.name << " ";
                details.ast->debugPrint(os) << " )";
                break;
            }

            case NodeType::Unused:
                break;
        }
        return os;
    }

    const Node& getTrueNode()
    {
        static const Node TrueNode(NodeType::Symbol, "true");
        return TrueNode;
    }

    const Node& getFalseNode()
    {
        static const Node FalseNode(NodeType::Symbol, "false");
        return FalseNode;
    }

    const Node& getNilNode()
    {
        static const Node NilNode(NodeType::Symbol, "nil");
        return NilNode;
    }

    const Node& getListNode()
    {
        static const Node ListNode(NodeType::Symbol, "list");
        return ListNode;
    }

    bool operator==(const Node& A, const Node& B)
    {
        if (A.m_type != B.m_type)  // should have the same types
            return false;

        if (A.m_type != NodeType::List)
            return A.m_value == B.m_value;
        return false;
    }

    bool operator<(const Node& A, const Node& B)
    {
        if (A.nodeType() != B.nodeType())
            return false;

        switch (A.nodeType())
        {
            case NodeType::Number:
                [[fallthrough]];
            case NodeType::Symbol:
                [[fallthrough]];
            case NodeType::String:
                return A.m_value < B.m_value;

            default:
                return false;
        }
    }
}
