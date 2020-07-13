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

inline std::string typeToString(const Node& node)
{
    // must have the same order as the enum class NodeType L17
    static const std::vector<std::string> nodetype_str = {
        "Symbol", "Capture", "GetField", "Keyword", "String", "Number", "List", "Closure"
    };

    if (node.nodeType() == NodeType::Symbol)
    {
        if (node.string() == "nil")
            return "Nil";
        return "Bool";
    }

    return nodetype_str[static_cast<int>(node.nodeType())];
}