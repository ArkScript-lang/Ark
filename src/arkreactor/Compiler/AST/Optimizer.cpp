#include <Ark/Compiler/AST/Optimizer.hpp>

#include <Ark/Exceptions.hpp>

namespace Ark::internal
{
    Optimizer::Optimizer(const unsigned debug) noexcept :
        Pass("Optimizer", debug), m_ast()
    {}

    void Optimizer::process(const Node& ast)
    {
        m_logger.traceStart("process");
        m_ast = ast;
        removeUnused();
        m_logger.traceEnd();

        m_logger.trace("AST after name pruning nodes");
        if (m_logger.shouldTrace())
            m_ast.debugPrint(std::cout) << '\n';
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

        countOccurences(m_ast);

        for (const auto& [name, uses] : m_sym_appearances)
            m_logger.debug("{} -> {}", name, uses);

        // logic: remove piece of code with only 1 reference, if they aren't function calls
        runOnGlobalScopeVars(m_ast, [this](const Node& node, Node& parent, const std::size_t idx) {
            const std::string name = node.constList()[1].string();
            // a variable was only declared and never used
            if (m_sym_appearances.contains(name) && m_sym_appearances[name] < 1)
            {
                m_logger.debug("Removing unused variable '{}'", name);
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
            else if (it->nodeType() == NodeType::Namespace)
            {
                m_logger.debug("Traversing namespace {}", it->arkNamespace().name);
                runOnGlobalScopeVars(*it->arkNamespace().ast, func);
            }
        }
    }

    void Optimizer::countOccurences(const Node& node)
    {
        if (node.nodeType() == NodeType::Symbol || node.nodeType() == NodeType::Capture)
        {
            auto [element, inserted] = m_sym_appearances.try_emplace(node.string(), 0);
            if (!inserted)
                element->second++;
        }
        else if (node.nodeType() == NodeType::List || node.nodeType() == NodeType::Field)
        {
            // iterate over children
            for (const auto& child : node.constList())
                countOccurences(child);
        }
        else if (node.nodeType() == NodeType::Namespace)
            countOccurences(*node.constArkNamespace().ast);
    }
}
