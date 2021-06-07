#include <Ark/Compiler/Node.hpp>

#include <termcolor.hpp>

namespace Ark::internal
{

    Node Node::TrueNode = Node("true");
    Node Node::FalseNode = Node("false");
    Node Node::NilNode = Node("nil");
    Node Node::ListNode = Node("list");

    void Node::init() noexcept
    {
        Node::TrueNode.setNodeType(NodeType::Symbol);
        Node::FalseNode.setNodeType(NodeType::Symbol);
        Node::NilNode.setNodeType(NodeType::Symbol);
        Node::ListNode.setNodeType(NodeType::Symbol);
    }

    Node::Node(long value) noexcept :
        m_type(NodeType::Number),
        m_value(static_cast<double>(value))
    {}

    Node::Node(double value) noexcept :
        m_type(NodeType::Number),
        m_value(value)
    {}

    Node::Node(const std::string& value) noexcept :
        m_type(NodeType::String),
        m_value(value)
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

    // -------------------------

    const std::string& Node::string() const noexcept
    {
        return mpark::get<std::string>(m_value);
    }

    double Node::number() const noexcept
    {
        return mpark::get<double>(m_value);
    }

    Keyword Node::keyword() const noexcept
    {
        return mpark::get<Keyword>(m_value);
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

    auto colors = std::vector({
        termcolor::blue,
        termcolor::red,
        termcolor::green,
        termcolor::cyan,
        termcolor::magenta
    });

    std::ostream& operator<<(std::ostream& os, const Node& N) noexcept
    {
        static int index = 0;

        switch(N.m_type)
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
            switch(N.keyword())
            {
                case Keyword::Fun:    os << "Fun";    break;
                case Keyword::Let:    os << "Let";    break;
                case Keyword::Mut:    os << "Mut";    break;
                case Keyword::Set:    os << "Set";    break;
                case Keyword::If:     os << "If";     break;
                case Keyword::While:  os << "While";  break;
                case Keyword::Begin:  os << "Begin";  break;
                case Keyword::Import: os << "Import"; break;
                case Keyword::Quote:  os << "Quote";  break;
                case Keyword::Del:    os << "Del";    break;
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
}
