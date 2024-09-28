#include <Ark/Compiler/NameResolutionPass.hpp>

#include <Ark/Exceptions.hpp>
#include <Ark/Utils.hpp>
#include <Ark/Builtins/Builtins.hpp>

#include <fmt/format.h>
#include <ranges>

namespace Ark::internal
{
    void ScopeResolver::Scope::add(const std::string& name, bool is_mutable)
    {
        m_vars.emplace(name, is_mutable);
    }

    std::optional<Variable> ScopeResolver::Scope::get(const std::string& name) const
    {
        if (const auto it = std::ranges::find(m_vars, name, &Variable::name); it != m_vars.end())
            return *it;
        return std::nullopt;
    }

    bool ScopeResolver::Scope::has(const std::string& name) const
    {
        return std::ranges::find(m_vars, name, &Variable::name) != m_vars.end();
    }

    ScopeResolver::ScopeResolver()
    {
        createNew();
    }

    void ScopeResolver::createNew()
    {
        m_scopes.emplace_back();
    }

    void ScopeResolver::removeLocalScope()
    {
        m_scopes.pop_back();
    }

    void ScopeResolver::registerInCurrent(const std::string& name, const bool is_mutable)
    {
        m_scopes.back().add(name, is_mutable);
    }

    std::optional<bool> ScopeResolver::isImmutable(const std::string& name) const
    {
        for (const auto& m_scope : std::ranges::reverse_view(m_scopes))
        {
            if (auto maybe = m_scope.get(name); maybe.has_value())
                return !maybe.value().is_mutable;
        }
        return std::nullopt;
    }

    bool ScopeResolver::isRegistered(const std::string& name) const
    {
        return std::ranges::any_of(
            m_scopes.rbegin(),
            m_scopes.rend(),
            [name](const Scope& scope) {
                return scope.has(name);
            });
    }

    bool ScopeResolver::isInScope(const std::string& name) const
    {
        return m_scopes.back().has(name);
    }

    NameResolutionPass::NameResolutionPass(const unsigned debug) :
        Pass("NameResolution", debug),
        m_ast()
    {
        for (const auto& builtin : Builtins::builtins)
            m_language_symbols.emplace(builtin.first);
        for (auto ope : Language::operators)
            m_language_symbols.emplace(ope);
        for (auto inst : Language::listInstructions)
            m_language_symbols.emplace(inst);

        m_language_symbols.emplace(Language::And);
        m_language_symbols.emplace(Language::Or);
        m_language_symbols.emplace(Language::SysArgs);
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

    void NameResolutionPass::addDefinedSymbol(const std::string& sym, const bool is_mutable)
    {
        m_defined_symbols.emplace(sym);
        m_scope_resolver.registerInCurrent(sym, is_mutable);
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
                        // the UpdateRef function calls kind get a special treatment, like let/mut/set,
                        // because we need to check for mutability errors
                        if (node.constList().size() > 1 && node.constList()[0].nodeType() == NodeType::Symbol &&
                            node.constList()[1].nodeType() == NodeType::Symbol)
                        {
                            const auto funcname = node.constList()[0].string();
                            const auto arg = node.constList()[1].string();

                            if (std::ranges::find(Language::UpdateRef, funcname) != Language::UpdateRef.end() && m_scope_resolver.isImmutable(arg).value_or(false))
                                throw CodeError(
                                    fmt::format("MutabilityError: Can not modify the constant list `{}' using `{}'", arg, funcname),
                                    node.filename(),
                                    node.constList()[1].line(),
                                    node.constList()[1].col(),
                                    arg);

                            // check that we aren't doing a (append! a a) nor a (concat! a a)
                            if (funcname == Language::AppendInPlace || funcname == Language::ConcatInPlace)
                            {
                                for (std::size_t i = 2, end = node.constList().size(); i < end; ++i)
                                {
                                    if (node.constList()[i].nodeType() == NodeType::Symbol && node.constList()[i].string() == arg)
                                        throw CodeError(
                                            fmt::format("MutabilityError: Can not {} the list `{}' to itself", funcname, arg),
                                            node.filename(),
                                            node.constList()[1].line(),
                                            node.constList()[1].col(),
                                            arg);
                                }
                            }
                        }

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
                {
                    const std::string& name = node.constList()[1].string();
                    if (m_language_symbols.contains(name))
                        throw CodeError(
                            fmt::format("Can not use a reserved identifier ('{}') as a {} name.", name, keyword == Keyword::Let ? "constant" : "variable"),
                            node.filename(),
                            node.constList()[1].line(),
                            node.constList()[1].col(),
                            name);

                    if (m_scope_resolver.isInScope(name) && keyword == Keyword::Let)
                        throw CodeError(
                            fmt::format("MutabilityError: Can not use 'let' to redefine variable `{}'", name),
                            node.filename(),
                            node.constList()[1].line(),
                            node.constList()[1].col(),
                            name);
                    else if (keyword == Keyword::Set)
                    {
                        const auto val = node.constList()[2].repr();

                        if (const auto mutability = m_scope_resolver.isImmutable(name); m_scope_resolver.isRegistered(name) &&
                            mutability.value_or(false))
                            throw CodeError(
                                fmt::format("MutabilityError: Can not set the constant `{}' to {}", name, val),
                                node.filename(),
                                node.constList()[1].line(),
                                node.constList()[1].col(),
                                name);
                    }
                    else
                        addDefinedSymbol(name, keyword != Keyword::Let);
                }
                break;

            case Keyword::Import:
                if (!node.constList().empty())
                    m_plugin_names.push_back(node.constList()[1].constList().back().string());
                break;

            case Keyword::Fun:
                // create a new scope to track variables
                m_scope_resolver.createNew();
                if (node.constList()[1].nodeType() == NodeType::List)
                {
                    for (const auto& child : node.constList()[1].constList())
                    {
                        if (child.nodeType() == NodeType::Capture)
                        {
                            // First, check that the capture is a defined symbol
                            if (!m_defined_symbols.contains(child.string()))
                            {
                                // we didn't find node in the defined symbol list, thus we can't capture node
                                throw CodeError(
                                    fmt::format("Can not capture {} because it is referencing an unbound variable.", child.string()),
                                    child.filename(),
                                    child.line(),
                                    child.col(),
                                    child.repr());
                            }
                            else if (!m_scope_resolver.isRegistered(child.string()))
                            {
                                throw CodeError(
                                    fmt::format("Can not capture {} because it is referencing a variable defined in an unreachable scope.", child.string()),
                                    child.filename(),
                                    child.line(),
                                    child.col(),
                                    child.repr());
                            }
                            addDefinedSymbol(child.string(), /* is_mutable= */ true);
                        }
                        else if (child.nodeType() == NodeType::Symbol)
                            addDefinedSymbol(child.string(), /* is_mutable= */ true);
                    }
                }
                if (node.constList().size() > 2)
                    visit(node.constList()[2]);
                m_scope_resolver.removeLocalScope();  // and remove it once the function has been compiled
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
