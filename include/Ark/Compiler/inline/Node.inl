template <typename T>
Node make_node(T&& value, std::size_t line, std::size_t col, const std::string& file)
{
    Node n(std::forward<T>(value));
    n.setPos(line, col);
    n.setFilename(file);
    return n;
}

inline Node make_node_list(std::size_t line, std::size_t col, const std::string& file)
{
    Node n(NodeType::List);
    n.setPos(line, col);
    n.setFilename(file);
    return n;
}

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

inline bool operator<(const Node& A, const Node& B)
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

inline bool operator!(const Node& A)
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


inline std::string typeToString(const Node& node) noexcept
{
    if (node.nodeType() == NodeType::Symbol)
    {
        if (node.string() == "nil")
            return "Nil";
        else if (node.string() == "true" || node.string() == "false")
            return "Bool";
    }

    const std::array<std::string, 11> nodetype_str = {
        "Symbol", "Capture", "GetField", "Keyword", "String",
        "Number", "List", "Closure", "Macro", "Spread", "Unused"
    };

    int c = static_cast<int>(node.nodeType());
    return (c < 11) ? nodetype_str[c] : "???";
}
