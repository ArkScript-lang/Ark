#include <Ark/Compiler/Optimizer.hpp>

namespace Ark
{
    using namespace internal;

    Optimizer::Optimizer(uint16_t options) noexcept :
        m_options(options)
    {}

    void Optimizer::feed(const Node& ast)
    {
        m_ast = ast;

        if (m_options & FeatureRemoveUnusedVars)
            remove_unused();
    }

    const Node& Optimizer::ast() const noexcept
    {
        return m_ast;
    }

    void Optimizer::remove_unused()
    {
        // do not handle non-list nodes
        if (m_ast.nodeType() != NodeType::List)
            return;

        runOnGlobalScopeVars(m_ast, [this](Node& node, Node& parent, int idx){
            m_sym_appearances[node.constList()[1].string()] = 0;
        });
        countOccurences(m_ast);

        // logic: remove piece of code with only 1 reference, if they aren't function calls
        runOnGlobalScopeVars(m_ast, [this](Node& node, Node& parent, int idx){
            std::string name = node.constList()[1].string();
            // a variable was only declared and never used
            if (m_sym_appearances.find(name) != m_sym_appearances.end() && m_sym_appearances[name] == 1
                && parent.list()[idx].list()[2].nodeType() != NodeType::List)
                parent.list().erase(parent.list().begin() + idx);  // erase the node from the list
        });
    }

    void Optimizer::runOnGlobalScopeVars(Node& node, const std::function<void(Node&, Node&, int)>& func)
    {
        int i = static_cast<int>(node.constList().size());
        // iterate only on the first level, using reverse iterators to avoid copy-delete-move to nowhere
        for (auto it = node.list().rbegin(); it != node.list().rend(); ++it)
        {
            i--;

            if (it->constList().size() > 0 && it->constList()[0].nodeType() == NodeType::Keyword)
            {
                Keyword kw = it->constList()[0].keyword();

                // eliminate nested begin blocks
                if (kw == Keyword::Begin)
                {
                    runOnGlobalScopeVars(*it, func);
                    // skip let/ mut detection
                    continue;
                }
                // check if it's a let/mut declaration
                else if (kw == Keyword::Let || kw == Keyword::Mut)
                    func(*it, node, i);
            }
        }
    }

    void Optimizer::countOccurences(Node& node)
    {
        if (node.nodeType() == NodeType::Symbol || node.nodeType() == NodeType::Capture)
        {
            std::string name = node.string();
            // check if it's the name of something declared in global scope
            if (m_sym_appearances.find(name) != m_sym_appearances.end())
                m_sym_appearances[name]++;
        }
        else if (node.nodeType() == NodeType::List)
        {
            // iterate over children
            for (std::size_t i = 0, end = node.constList().size(); i != end; ++i)
                countOccurences(node.list()[i]);
        }
    }
}