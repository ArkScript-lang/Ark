#include <Ark/Parser/Node.hpp>

#include <termcolor.hpp>

namespace Ark::internal
{
    Node::Node(int value) :
        m_type(NodeType::Number),
        m_value(static_cast<double>(value))
    {}

    Node::Node(double value) :
        m_type(NodeType::Number),
        m_value(value)
    {}
    
    Node::Node(const std::string& value) :
        m_type(NodeType::String),
        m_value(value)
    {}

    Node::Node(Keyword value) :
        m_type(NodeType::Keyword),
        m_value(value)
    {}

    Node::Node(NodeType type) :
        m_type(type)
    {}

    // -------------------------

    const std::string& Node::string() const
    {
        return std::get<std::string>(m_value);
    }

    double Node::number() const
    {
        return std::get<double>(m_value);
    }

    Keyword Node::keyword() const
    {
        return std::get<Keyword>(m_value);
    }

    // -------------------------

    void Node::push_back(const Node& node)
    {
        m_list.push_back(node);
    }

    std::vector<Node>& Node::list()
    {
        return m_list;
    }

    const std::vector<Node>& Node::const_list() const
    {
        return m_list;
    }

    // -------------------------

    NodeType Node::nodeType() const
    {
        return m_type;
    }

    void Node::setNodeType(NodeType type)
    {
        m_type = type;
    }

    void Node::setString(const std::string& value)
    {
        m_value = value;
    }

    void Node::setNumber(double value)
    {
        m_value = value;
    }

    void Node::setKeyword(Keyword kw)
    {
        m_value = kw;
    }

    // -------------------------

    void Node::setPos(std::size_t line, std::size_t col)
    {
        m_line = line;
        m_col = col;
    }

    std::size_t Node::line() const
    {
        return m_line;
    }
    
    std::size_t Node::col() const
    {
        return m_col;
    }

    // -------------------------

    auto colors = std::vector({
        termcolor::blue,
        termcolor::red,
        termcolor::green,
        termcolor::cyan,
        termcolor::magenta
    });

    std::ostream& operator<<(std::ostream& os, const Node& N)
    {
        static int index = 0;

        switch(N.m_type)
        {
        case NodeType::String:
        case NodeType::Symbol:
            os << N.string();
            break;

        case NodeType::Number:
            os << N.number();
            break;

        case NodeType::List:
        {
            os << colors[index % colors.size()] << "( " << termcolor::reset;
            index++;
            for (auto& t: N.m_list)
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

        default:
            os << "~\\._./~";
            break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Nodes& N)
    {
        os << "( ";
        for (auto& t: N)
            os << t << " ";
        os << ")";

        return os;
    }
}
