#include <Ark/Compiler/AST/Optimizer.hpp>

#include <Ark/Utils/Utils.hpp>

namespace Ark::internal
{
    Optimizer::Optimizer(const unsigned debug) noexcept :
        Pass("Optimizer", debug), m_ast()
    {}

    void Optimizer::process(const Node& ast)
    {
        // do not handle non-list nodes
        if (ast.nodeType() != NodeType::List)
            return;
        m_ast = ast;

        m_logger.traceStart("process");
        countAndPruneDeadCode(m_ast);

        // logic: remove piece of code with only 1 reference, if they aren't function calls
        pruneUnusedGlobalVariables(m_ast);
        m_logger.traceEnd();

        m_logger.debug("AST after name pruning nodes");
        if (m_logger.shouldDebug())
            m_ast.debugPrint(std::cout) << '\n';
    }

    const Node& Optimizer::ast() const noexcept
    {
        return m_ast;
    }

    void Optimizer::countAndPruneDeadCode(Node& node)
    {
        if (node.nodeType() == NodeType::Symbol || node.nodeType() == NodeType::Capture)
        {
            auto [element, inserted] = m_sym_appearances.try_emplace(node.string(), 0);
            if (!inserted)
                element->second++;
        }
        else if (node.nodeType() == NodeType::Field)
        {
            for (auto& child : node.list())
                countAndPruneDeadCode(child);
        }
        else if (node.nodeType() == NodeType::List)
        {
            // FIXME: very primitive removal of (if true/false ...) and (while false ...)
            if (node.constList().size() >= 3 && node.constList().front().nodeType() == NodeType::Keyword &&
                (node.constList().front().keyword() == Keyword::If || node.constList().front().keyword() == Keyword::While))
            {
                const auto keyword = node.constList().front().keyword();
                const auto condition = node.constList()[1];
                const auto body = node.constList()[2];

                if (condition.nodeType() == NodeType::Symbol && condition.string() == "false")
                {
                    // replace the node by an Unused, it is either a (while cond block) or (if cond then)
                    if (node.constList().size() == 3)
                        node = Node(NodeType::Unused);
                    else  // it is a (if cond then else)
                    {
                        const auto back = node.constList().back();
                        node = back;
                    }
                }
                // only update '(if true then [else])' to 'then'
                else if (keyword == Keyword::If && condition.nodeType() == NodeType::Symbol && condition.string() == "true")
                    node = body;

                // do not try to iterate on the child nodes as they do not exist anymore,
                // we performed some optimization that squashed them.
                if (!node.isListLike())
                    return;
            }

            // iterate over children
            for (auto& child : node.list())
                countAndPruneDeadCode(child);
        }
        else if (node.nodeType() == NodeType::Namespace)
            countAndPruneDeadCode(*node.arkNamespace().ast);
    }

    void Optimizer::pruneUnusedGlobalVariables(Node& node)
    {
        for (auto& child : node.list())
        {
            if (child.nodeType() == NodeType::List && !child.constList().empty() &&
                child.constList()[0].nodeType() == NodeType::Keyword)
            {
                const Keyword kw = child.constList()[0].keyword();

                // eliminate nested begin blocks
                if (kw == Keyword::Begin)
                {
                    pruneUnusedGlobalVariables(child);
                    // skip let/ mut detection
                    continue;
                }

                // check if it's a let/mut declaration and perform removal
                if (kw == Keyword::Let || kw == Keyword::Mut)
                {
                    const std::string name = child.constList()[1].string();
                    // a variable was only declared and never used
                    if (m_sym_appearances.contains(name) && m_sym_appearances[name] < 1)
                    {
                        m_logger.debug("Removing unused variable '{}'", name);
                        // erase the node by turning it to an Unused node
                        child = Node(NodeType::Unused);
                    }
                    else if (child.comment().find("@deprecated") != std::string::npos)
                    {
                        std::string advice;
                        const std::vector<std::string> vec = Utils::splitString(child.comment(), '\n');
                        if (auto result = std::ranges::find_if(vec, [](const std::string& line) {
                                return line.find("@deprecated") != std::string::npos;
                            });
                            result != vec.end())
                        {
                            advice = result->substr(result->find("@deprecated") + 11);
                            Utils::trimWhitespace(advice);
                        }

                        m_logger.warn(
                            "Using a deprecated {} `{}'.{}",
                            child.constList()[2].nodeType() == NodeType::List &&
                                    !child.constList()[2].constList().empty() &&
                                    child.constList()[2].constList()[0].nodeType() == NodeType::Keyword &&
                                    child.constList()[2].constList()[0].keyword() == Keyword::Fun
                                ? "function"
                                : "value",
                            name,
                            advice.empty() ? "" : " " + advice);
                    }
                }
            }
            else if (child.nodeType() == NodeType::Namespace)
                pruneUnusedGlobalVariables(*child.arkNamespace().ast);
        }
    }
}
