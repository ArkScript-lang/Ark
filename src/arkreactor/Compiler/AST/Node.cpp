#include <Ark/Compiler/Common.hpp>
#include <Ark/Compiler/AST/Node.hpp>

#include <Ark/Exceptions.hpp>

#include <fmt/core.h>

namespace Ark::internal
{
    Node::Node(const NodeType node_type, const std::string& value) :
        m_type(node_type), m_value(value)
    {}

    Node::Node(const NodeType node_type) :
        m_type(node_type)
    {
        if (m_type == NodeType::List || m_type == NodeType::Macro || m_type == NodeType::Field)
            m_value = std::vector<Node>();
    }

    Node::Node(double value) :
        m_type(NodeType::Number), m_value(value)
    {}

    Node::Node(const long value) :
        m_type(NodeType::Number), m_value(static_cast<double>(value))
    {}

    Node::Node(const int value) :
        m_type(NodeType::Number), m_value(static_cast<double>(value))
    {}

    Node::Node(Keyword value) :
        m_type(NodeType::Keyword), m_value(value)
    {}

    Node::Node(const std::vector<Node>& nodes) :
        m_type(NodeType::List), m_value(nodes)
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

    void Node::setNodeType(const NodeType type) noexcept
    {
        m_type = type;
    }

    void Node::setString(const std::string& value) noexcept
    {
        m_value = value;
    }

    void Node::setPos(const std::size_t line, const std::size_t col) noexcept
    {
        m_line = line;
        m_col = col;
    }

    void Node::setFilename(const std::string& filename) noexcept
    {
        m_filename = filename;
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

    std::size_t Node::line() const noexcept
    {
        return m_line;
    }

    std::size_t Node::col() const noexcept
    {
        return m_col;
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
                data += "(";
                for (std::size_t i = 0, end = constList().size(); i < end; ++i)
                {
                    data += constList()[i].repr();
                    if (i < end - 1)
                        data += " ";
                }
                data += ")";
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
                data += "($ ";
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

            case NodeType::Unused:
                os << "Unused:" << string();
                break;

            default:
                os << "~\\._./~";
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

    // todo: do we really need all those operators? maybe for macros?
    bool operator==(const Node& A, const Node& B)
    {
        if (A.m_type != B.m_type)  // should have the same types
            return false;

        if (A.m_type != NodeType::List)
            return A.m_value == B.m_value;
        else
            throw TypeError("Can not compare lists");
    }

    bool operator<(const Node& A, const Node& B)
    {
        if (A.nodeType() != B.nodeType())
            return (static_cast<int>(A.nodeType()) - static_cast<int>(B.nodeType())) < 0;

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

    bool operator!(const Node& A)
    {
        switch (A.nodeType())
        {
            case NodeType::Field:
                [[fallthrough]];
            case NodeType::List:
                return A.constList().empty();

            case NodeType::Number:
                return !A.number();

            case NodeType::Capture:
                [[fallthrough]];
            case NodeType::String:
                return A.string().size() == 0;

            case NodeType::Symbol:
            {
                if (A.string() == "true")
                    return false;
                if (A.string() == "false" || A.string() == "nil")
                    return true;
            }
                return false;

            default:
                return false;
        }
    }
}
