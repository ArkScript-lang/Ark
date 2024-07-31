#include <Ark/Compiler/AST/Optimizer.hpp>

namespace Ark::internal
{
    Optimizer::Optimizer(const unsigned debug) noexcept :
        Pass("Optimizer", debug), m_ast()
    {}

    void Optimizer::process(const Node& ast)
    {
        m_ast = ast;
        // FIXME activate this removeUnused();
    }

    const Node& Optimizer::ast() const noexcept
    {
        return m_ast;
    }

    void Optimizer::throwOptimizerError(const std::string& message, const Node& node)
    {
        throw CodeError(message, node.filename(), node.line(), node.col(), node.repr());
    }

    void Optimizer::removeUnused()
    {
        // do not handle non-list nodes
        if (m_ast.nodeType() != NodeType::List)
            return;

        runOnGlobalScopeVars(m_ast, [this](const Node& node, Node& parent [[maybe_unused]], int idx [[maybe_unused]]) {
            m_sym_appearances[node.constList()[1].string()] = 0;
        });
        countOccurences(m_ast);

        // logic: remove piece of code with only 1 reference, if they aren't function calls
        runOnGlobalScopeVars(m_ast, [this](const Node& node, Node& parent, const std::size_t idx) {
            std::string name = node.constList()[1].string();
            // a variable was only declared and never used
            if (m_sym_appearances.contains(name) && m_sym_appearances[name] == 1 && parent.list()[idx].list()[2].nodeType() != NodeType::List)
            {
                logDebug("Removing unused variable '{}'", name);
                // erase the node from the list
                parent.list().erase(parent.list().begin() + static_cast<std::vector<Node>::difference_type>(idx));
            }
        });
    }

    void Optimizer::runOnGlobalScopeVars(Node& node, const std::function<void(Node&, Node&, std::size_t)>& func)
    {
        auto i = node.constList().size();
        // iterate only on the first level, using reverse iterators to avoid copy-delete-move to nowhere
        for (auto it = node.list().rbegin(); it != node.list().rend(); ++it)
        {
            i--;

            if (it->nodeType() == NodeType::List && !it->constList().empty() &&
                it->constList()[0].nodeType() == NodeType::Keyword)
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
                if (kw == Keyword::Let || kw == Keyword::Mut)
                    func(*it, node, i);
            }
        }
    }

    void Optimizer::countOccurences(Node& node)
    {
        if (node.nodeType() == NodeType::Symbol || node.nodeType() == NodeType::Capture)
        {
            // check if it's the name of something declared in global scope
            if (const auto name = node.string(); m_sym_appearances.contains(name))
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
