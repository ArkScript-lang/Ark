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

        run_on_global_scope_vars(m_ast, [this](Node& node, Node& parent, int idx){
            m_symAppearances[node.const_list()[1].string()] = 0;
        });
        count_occurences(m_ast);

        // logic: remove piece of code with only 1 reference
        run_on_global_scope_vars(m_ast, [this](Node& node, Node& parent, int idx){
            std::string name = node.const_list()[1].string();
            // a variable was only declared and never used
            if (m_symAppearances.find(name) != m_symAppearances.end() && m_symAppearances[name] == 1)
                parent.list().erase(parent.list().begin() + idx);  // erase the node from the list
        });
    }

    void Optimizer::run_on_global_scope_vars(Node& node, const std::function<void(Node&, Node&, int)>& func)
    {
        int i = static_cast<int>(node.const_list().size());
        // iterate only on the first level, using reverse iterators to avoid copy-delete-move to nowhere
        for (auto it=node.list().rbegin(); it != node.list().rend(); ++it)
        {
            i--;

            if (it->const_list().size() > 0 && it->const_list()[0].nodeType() == NodeType::Keyword)
            {
                Keyword kw = it->const_list()[0].keyword();

                // eliminate nested begin blocks
                if (kw == Keyword::Begin)
                {
                    run_on_global_scope_vars(*it, func);
                    // skip let/ mut detection
                    continue;
                }
                // check if it's a let/mut declaration
                else if (kw == Keyword::Let || kw == Keyword::Mut)
                    func(*it, node, i);
            }
        }
    }

    void Optimizer::count_occurences(Node& node)
    {
        if (node.nodeType() == NodeType::Symbol || node.nodeType() == NodeType::Capture)
        {
            std::string name = node.string();
            // check if it's the name of something declared in global scope
            if (m_symAppearances.find(name) != m_symAppearances.end())
                m_symAppearances[name]++;
        }
        else if (node.nodeType() == NodeType::List)
        {
            // iterate over children
            for (std::size_t i = 0, end = node.const_list().size(); i != end; ++i)
                count_occurences(node.list()[i]);
        }
    }
}