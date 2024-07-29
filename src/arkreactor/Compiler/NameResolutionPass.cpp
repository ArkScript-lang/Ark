#include <Ark/Compiler/NameResolutionPass.hpp>

#include <Ark/Exceptions.hpp>
#include <Ark/Utils.hpp>
#include <Ark/Builtins/Builtins.hpp>

#include <fmt/format.h>

namespace Ark::internal
{
    // todo add scope resolution to this pass
    NameResolutionPass::NameResolutionPass(const unsigned debug) :
        Pass("NameResolution", debug),
        m_ast()
    {
        for (const auto& builtin : Builtins::builtins)
            m_language_symbols.emplace(builtin.first);
        for (auto ope : operators)
            m_language_symbols.emplace(ope);
        for (auto inst : listInstructions)
            m_language_symbols.emplace(inst);
    }

    void NameResolutionPass::process(const Node& ast)
    {
        m_ast = ast;
        visit(ast);
        checkForUndefinedSymbol();
    }

    const Node& NameResolutionPass::ast() const noexcept
    {
        return m_ast;
    }

    void NameResolutionPass::addDefinedSymbol(const std::string& sym)
    {
        m_defined_symbols.emplace(sym);
    }

    void NameResolutionPass::visit(const Node& node)
    {
        switch (node.nodeType())
        {
            case NodeType::Symbol:
                addSymbolNode(node);
                break;

            case NodeType::Field:
                for (const auto& child : node.constList())
                    addSymbolNode(child);
                break;

            case NodeType::List:
                if (!node.constList().empty())
                {
                    if (node.constList()[0].nodeType() == NodeType::Keyword)
                        visitKeyword(node, node.constList()[0].keyword());
                    else
                    {
                        // function calls
                        for (const auto& child : node.constList())
                            visit(child);
                    }
                }
                break;

            default:
                break;
        }
    }

    void NameResolutionPass::visitKeyword(const Node& node, const Keyword keyword)
    {
        switch (keyword)
        {
            case Keyword::Set:
                [[fallthrough]];
            case Keyword::Let:
                [[fallthrough]];
            case Keyword::Mut:
                // first, visit the value, then register the symbol
                // this allows us to detect things like (let foo (fun (&foo) ()))
                if (node.constList().size() > 2)
                    visit(node.constList()[2]);
                if (node.constList().size() > 1 && node.constList()[1].nodeType() == NodeType::Symbol)
                    addDefinedSymbol(node.constList()[1].string());
                break;

            case Keyword::Import:
                if (!node.constList().empty())
                    m_plugin_names.push_back(node.constList()[1].constList().back().string());
                break;

            case Keyword::Fun:
                if (node.constList()[1].nodeType() == NodeType::List)
                {
                    for (const auto& child : node.constList()[1].constList())
                    {
                        if (child.nodeType() == NodeType::Capture)
                        {
                            // First, check that the capture is a defined symbol
                            // TODO add a scope thing to see if we are capturing something in scope
                            if (!m_defined_symbols.contains(child.string()))
                            {
                                // we didn't find node in the defined symbol list, thus we can't capture node
                                throw CodeError(
                                    fmt::format("Can not capture {} because node is referencing an unbound variable.", child.string()),
                                    child.filename(),
                                    child.line(),
                                    child.col(),
                                    child.repr());
                            }
                            addDefinedSymbol(child.string());
                        }
                        else if (child.nodeType() == NodeType::Symbol)
                            addDefinedSymbol(child.string());
                    }
                }
                if (node.constList().size() > 2)
                    visit(node.constList()[2]);
                break;

            default:
                for (const auto& child : node.constList())
                    visit(child);
                break;
        }
    }

    void NameResolutionPass::addSymbolNode(const Node& symbol)
    {
        const std::string& name = symbol.string();

        // we don't accept builtins/operators as a user symbol
        if (m_language_symbols.contains(name))
            return;

        const auto it = std::ranges::find_if(m_symbol_nodes, [&name](const Node& sym_node) -> bool {
            return sym_node.string() == name;
        });
        if (it == m_symbol_nodes.end())
            m_symbol_nodes.push_back(symbol);
    }

    bool NameResolutionPass::mayBeFromPlugin(const std::string& name) const noexcept
    {
        std::string splitted = Utils::splitString(name, ':')[0];
        const auto it = std::ranges::find_if(
            m_plugin_names,
            [&splitted](const std::string& plugin) -> bool {
                return plugin == splitted;
            });
        return it != m_plugin_names.end();
    }

    void NameResolutionPass::checkForUndefinedSymbol() const
    {
        for (const auto& sym : m_symbol_nodes)
        {
            const auto& str = sym.string();
            const bool is_plugin = mayBeFromPlugin(str);

            if (!m_defined_symbols.contains(str) && !is_plugin)
            {
                std::string message;

                const std::string suggestion = offerSuggestion(str);
                if (suggestion.empty())
                    message = fmt::format(R"(Unbound variable error "{}" (variable is used but not defined))", str);
                else
                    message = fmt::format(R"(Unbound variable error "{}" (did you mean "{}"?))", str, suggestion);

                throw CodeError(message, sym.filename(), sym.line(), sym.col(), sym.repr());
            }
        }
    }

    std::string NameResolutionPass::offerSuggestion(const std::string& str) const
    {
        std::string suggestion;
        // our suggestion shouldn't require more than half the string to change
        std::size_t suggestion_distance = str.size() / 2;

        for (const std::string& symbol : m_defined_symbols)
        {
            const std::size_t current_distance = Utils::levenshteinDistance(str, symbol);
            if (current_distance <= suggestion_distance)
            {
                suggestion_distance = current_distance;
                suggestion = symbol;
            }
        }

        return suggestion;
    }
}
