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
        // otherwise, add the symbol, and return its id in the table
        if (std::ranges::find(m_defined_symbols, sym) == m_defined_symbols.end())
            m_defined_symbols.push_back(sym);
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
                if (node.constList().size() > 1 && node.constList()[1].nodeType() == NodeType::Symbol)
                    addDefinedSymbol(node.constList()[1].string());
                if (node.constList().size() > 2)
                    visit(node.constList()[2]);
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
                        if (child.nodeType() == NodeType::Capture || child.nodeType() == NodeType::Symbol)
                        {
                            // FIXME first check that the capture is a defined symbol
                            // if (std::ranges::find(m_defined_symbols, node.string()) == m_defined_symbols.end())
                            // {
                            //     // we didn't find node in the defined symbol list, thus we can't capture node
                            //     throwCompilerError("Can not capture " + node.string() + " because node is referencing an unbound variable.", node);
                            // }

                            addDefinedSymbol(child.string());
                        }
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

            if (auto it = std::ranges::find(m_defined_symbols, str); it == m_defined_symbols.end() && !is_plugin)
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
