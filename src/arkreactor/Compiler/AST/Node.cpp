#include <Ark/Compiler/AST/Node.hpp>

#include <termcolor/proxy.hpp>

#include <Ark/Exceptions.hpp>

namespace Ark::internal
{
    // Static methods.
    const Node& Node::getTrueNode()
    {
        static const Node TrueNode { "true", NodeType::Symbol };
        return TrueNode;
    }

    const Node& Node::getFalseNode()
    {
        static const Node FalseNode { "false", NodeType::Symbol };
        return FalseNode;
    }

    const Node& Node::getNilNode()
    {
        static const Node NilNode { "nil", NodeType::Symbol };
        return NilNode;
    }

    const Node& Node::getListNode()
    {
        static const Node ListNode { "list", NodeType::Symbol };
        return ListNode;
    }

    // Normal Methods
    Node::Node(long value) noexcept :
        m_type(NodeType::Number),
        m_value(static_cast<double>(value))
    {}

    Node::Node(double value) noexcept :
        m_type(NodeType::Number),
        m_value(value)
    {}

    Node::Node(const std::string& value, NodeType const& type) noexcept :
        m_type(type),
        m_value(value)
    {}

    Node::Node(const std::string& value) noexcept :
        Node(value, NodeType::String)
    {}

    Node::Node(Keyword value) noexcept :
        m_type(NodeType::Keyword),
        m_value(value)
    {}

    Node::Node(NodeType type) noexcept :
        m_type(type)
    {}

    Node::Node(const Node& other) noexcept :
        m_type(other.m_type),
        m_value(other.m_value),
        m_list(other.m_list),
        m_line(other.m_line),
        m_col(other.m_col),
        m_filename(other.m_filename)
    {}

    Node& Node::operator=(Node other) noexcept
    {
        swap(other);
        return *this;
    }

    void Node::swap(Node& other) noexcept
    {
        using std::swap;

        swap(m_type, other.m_type);
        swap(m_value, other.m_value);
        swap(m_list, other.m_list);
        swap(m_line, other.m_line);
        swap(m_col, other.m_col);
        swap(m_filename, other.m_filename);
    }

    // -------------------------

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

    // -------------------------

    void Node::push_back(const Node& node) noexcept
    {
        m_list.push_back(node);
    }

    std::vector<Node>& Node::list() noexcept
    {
        return m_list;
    }

    const std::vector<Node>& Node::constList() const noexcept
    {
        return m_list;
    }

    // -------------------------

    NodeType Node::nodeType() const noexcept
    {
        return m_type;
    }

    void Node::setNodeType(NodeType type) noexcept
    {
        m_type = type;
    }

    void Node::setString(const std::string& value) noexcept
    {
        m_value = value;
    }

    void Node::setNumber(double value) noexcept
    {
        m_value = value;
    }

    void Node::setKeyword(Keyword kw) noexcept
    {
        m_value = kw;
    }

    // -------------------------

    void Node::setPos(std::size_t line, std::size_t col) noexcept
    {
        m_line = line;
        m_col = col;
    }

    void Node::setFilename(const std::string& filename) noexcept
    {
        m_filename = filename;
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

    // -------------------------

    auto colors = std::vector(
        { termcolor::blue,
          termcolor::red,
          termcolor::green,
          termcolor::cyan,
          termcolor::magenta });

    void swap(Node& lhs, Node& rhs) noexcept
    {
        lhs.swap(rhs);
    }
    std::ostream& operator<<(std::ostream& os, const Node& N) noexcept
    {
        static int index = 0;

        switch (N.m_type)
        {
            case NodeType::String:
                os << '"' << N.string() << '"';
                break;

            case NodeType::Symbol:
                os << "(Symbol) " << N.string();
                break;

            case NodeType::Capture:
                os << "(Capture) " << N.string();
                break;

            case NodeType::GetField:
                os << "(GetField) " << N.string();
                break;

            case NodeType::Number:
                os << N.number();
                break;

            case NodeType::List:
            {
                os << colors[index % colors.size()] << "( " << termcolor::reset;
                index++;
                for (auto& t : N.m_list)
                    os << t << " ";
                index--;
                os << colors[index % colors.size()] << ")" << termcolor::reset;
                break;
            }

            case NodeType::Closure:
                os << "Closure";
                break;

            case NodeType::Keyword:
                switch (N.keyword())
                {
                    case Keyword::Fun: os << "Fun"; break;
                    case Keyword::Let: os << "Let"; break;
                    case Keyword::Mut: os << "Mut"; break;
                    case Keyword::Set: os << "Set"; break;
                    case Keyword::If: os << "If"; break;
                    case Keyword::While: os << "While"; break;
                    case Keyword::Begin: os << "Begin"; break;
                    case Keyword::Import: os << "Import"; break;
                    case Keyword::Quote: os << "Quote"; break;
                    case Keyword::Del: os << "Del"; break;
                }
                break;

            case NodeType::Macro:
            {
                os << colors[index % colors.size()] << "( " << termcolor::reset << "Macro ";
                index++;
                for (auto& t : N.m_list)
                    os << t << " ";
                index--;
                os << colors[index % colors.size()] << ")" << termcolor::reset;
                break;
            }

            case NodeType::Spread:
                os << "(Spread) " << N.string();
                break;

            case NodeType::Unused:
                os << "(Unused)";
                break;

            default:
                os << "~\\._./~";
                break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const std::vector<Node>& N) noexcept
    {
        os << "( ";
        for (auto& t : N)
            os << t << " ";
        os << ")";

        return os;
    }

    bool operator==(const Node& A, const Node& B)
    {
        if (A.m_type != B.m_type)  // should have the same types
            return false;

        if (A.m_type != NodeType::List &&
            A.m_type != NodeType::Closure)
            return A.m_value == B.m_value;

        if (A.m_type == NodeType::List)
            throw TypeError("Can not compare lists");

        // any other type => false (here, Closure)
        return false;
    }

    bool operator<(const Node& A, const Node& B)
    {
        if (A.nodeType() != B.nodeType())
            return (static_cast<int>(A.nodeType()) - static_cast<int>(B.nodeType())) < 0;

        switch (A.nodeType())
        {
            case NodeType::Number:
            case NodeType::Symbol:
            case NodeType::String:
                return A.m_value < B.m_value;

            case NodeType::List:
                return A.m_list < B.m_list;

            default:
                return false;
        }
    }

    bool operator!(const Node& A)
    {
        switch (A.nodeType())
        {
            case NodeType::List:
                return A.constList().empty();

            case NodeType::Number:
                return !A.number();

            case NodeType::GetField:
            case NodeType::Capture:
            case NodeType::String:
                return A.string().size() == 0;

            case NodeType::Symbol:
                if (A.string() == "true")
                    return false;
                else if (A.string() == "false" || A.string() == "nil")
                    return true;
                return false;

            default:
                return false;
        }
    }
}
