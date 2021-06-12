inline Node* MacroProcessor::findNearestMacro(const std::string& name)
{
    if (m_macros.empty())
        return nullptr;

    for (auto it = m_macros.rbegin(); it != m_macros.rend(); ++it)
    {
        if (it->size() != 0)
        {
            if (auto res = it->find(name); res != it->end())
                return &res->second;
        }
    }
    return nullptr;
}

inline void MacroProcessor::deleteNearestMacro(const std::string& name)
{
    if (m_macros.empty())
        return;

    for (auto it = m_macros.rbegin(); it != m_macros.rend(); ++it)
    {
        if (it->size() != 0)
        {
            if (auto res = it->find(name); res != it->end())
            {
                // stop right here because we found one matching macro
                it->erase(res);
                return;
            }
        }
    }
}

inline bool MacroProcessor::isPredefined(const std::string& symbol)
{
    auto it = std::find(
        m_predefined_macros.begin(),
        m_predefined_macros.end(),
        symbol
    );

    return it != m_predefined_macros.end();
}

inline void MacroProcessor::recurApply(Node& node)
{
    applyMacro(node);

    if (node.nodeType() == NodeType::List)
    {
        for (std::size_t i = 0; i < node.list().size(); ++i)
            recurApply(node.list()[i]);
    }
}

inline bool MacroProcessor::hadBegin(const Node& node)
{
    return node.nodeType() == NodeType::List &&
            node.constList().size() > 0 &&
            node.constList()[0].nodeType() == NodeType::Keyword &&
            node.constList()[0].keyword() == Keyword::Begin;
}

inline void MacroProcessor::removeBegin(Node& node, std::size_t& i)
{
    if (node.nodeType() == NodeType::List && node.list()[i].nodeType() == NodeType::List && node.list()[i].list().size() > 0)
    {
        Node lst = node.constList()[i];
        Node first = lst.constList()[0];

        if (first.nodeType() == NodeType::Keyword && first.keyword() == Keyword::Begin)
        {
            // std::cout << "before " << node.constList()[i] << "\n";
            // std::cout << node << "\n";
            std::size_t previous = i;

            for (std::size_t block_idx = 1, end = lst.constList().size(); block_idx < end; ++block_idx)
                node.list().insert(node.constList().begin() + i + block_idx, lst.list()[block_idx]);

            i += lst.constList().size() - 2;  // -2 instead of -1 because it get incremented right after
            node.list().erase(node.constList().begin() + previous);
            // std::cout << "after " << node.constList()[i] << "\n";
            // std::cout << node << "\n\n";
        }
    }
}