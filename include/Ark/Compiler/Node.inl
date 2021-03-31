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
            return A.const_list().empty();

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
    // must have the same order as the enum class NodeType L17
    static const std::vector<std::string> nodetype_str = {
        "Symbol", "Capture", "GetField", "Keyword", "String",
        "Number", "List", "Closure", "Macro", "Spread"
    };

    if (node.nodeType() == NodeType::Symbol)
    {
        if (node.string() == "nil")
            return "Nil";
        else if (node.string() == "true" || node.string() == "false")
            return "Bool";
    }

    return nodetype_str[static_cast<int>(node.nodeType())];
}