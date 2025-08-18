#include <Ark/Compiler/NameResolution/NameResolutionPass.hpp>

#include <Ark/Error/Exceptions.hpp>
#include <Ark/Utils/Utils.hpp>
#include <Ark/Builtins/Builtins.hpp>

namespace Ark::internal
{
    NameResolutionPass::NameResolutionPass(const unsigned debug) :
        Pass("NameResolution", debug)
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
        m_language_symbols.emplace(Language::SysProgramName);
    }

    void NameResolutionPass::process(const Node& ast)
    {
        m_logger.traceStart("process");

        m_ast = ast;
        visit(m_ast, /* register_declarations= */ true);

        m_logger.traceEnd();

        m_logger.trace("AST after name resolution");
        if (m_logger.shouldTrace())
            m_ast.debugPrint(std::cout) << '\n';

        m_logger.traceStart("checkForUndefinedSymbol");
        checkForUndefinedSymbol();
        m_logger.traceEnd();
    }

    const Node& NameResolutionPass::ast() const noexcept
    {
        return m_ast;
    }

    std::string NameResolutionPass::addDefinedSymbol(const std::string& sym, const bool is_mutable)
    {
        const std::string fully_qualified_name = m_scope_resolver.registerInCurrent(sym, is_mutable);
        m_defined_symbols.emplace(fully_qualified_name);
        return fully_qualified_name;
    }

    void NameResolutionPass::visit(Node& node, const bool register_declarations)
    {
        switch (node.nodeType())
        {
            case NodeType::Symbol:
            {
                const std::string old_name = node.string();
                updateSymbolWithFullyQualifiedName(node);
                addSymbolNode(node, old_name);
                break;
            }

            case NodeType::Field:
                for (std::size_t i = 0, end = node.list().size(); i < end; ++i)
                {
                    Node& child = node.list()[i];

                    if (i == 0)
                    {
                        const std::string old_name = child.string();
                        // in case of field, no need to check if we can fully qualify names
                        child.setString(m_scope_resolver.getFullyQualifiedNameInNearestScope(old_name));
                        addSymbolNode(child, old_name);
                    }
                    else
                        addSymbolNode(child);
                }
                break;

            case NodeType::List:
                if (!node.constList().empty())
                {
                    if (node.constList()[0].nodeType() == NodeType::Keyword)
                        visitKeyword(node, node.constList()[0].keyword(), register_declarations);
                    else
                    {
                        // function calls
                        // the UpdateRef function calls kind get a special treatment, like let/mut/set,
                        // because we need to check for mutability errors
                        if (node.constList().size() > 1 && node.constList()[0].nodeType() == NodeType::Symbol &&
                            node.constList()[1].nodeType() == NodeType::Symbol && register_declarations)
                        {
                            const auto funcname = node.constList()[0].string();
                            const auto arg = node.constList()[1].string();

                            if (std::ranges::find(Language::UpdateRef, funcname) != Language::UpdateRef.end() && m_scope_resolver.isImmutable(arg).value_or(false))
                                throw CodeError(
                                    fmt::format("MutabilityError: Can not modify the constant list `{}' using `{}'", arg, funcname),
                                    CodeErrorContext(
                                        node.filename(),
                                        node.constList()[1].position(),
                                        arg));

                            // check that we aren't doing a (append! a a) nor a (concat! a a)
                            if (funcname == Language::AppendInPlace || funcname == Language::ConcatInPlace)
                            {
                                for (std::size_t i = 2, end = node.constList().size(); i < end; ++i)
                                {
                                    if (node.constList()[i].nodeType() == NodeType::Symbol && node.constList()[i].string() == arg)
                                        throw CodeError(
                                            fmt::format("MutabilityError: Can not {} the list `{}' to itself", funcname, arg),
                                            CodeErrorContext(
                                                node.filename(),
                                                node.constList()[1].position(),
                                                arg));
                                }
                            }
                        }

                        for (auto& child : node.list())
                            visit(child, register_declarations);
                    }
                }
                break;

            case NodeType::Namespace:
            {
                auto& namespace_ = node.arkNamespace();
                // no need to guard createNewNamespace with an if (register_declarations), we want to keep the namespace node
                // (which will get ignored by the compiler, that only uses its AST), so that we can (re)construct the
                // scopes correctly
                m_scope_resolver.createNewNamespace(namespace_.name, namespace_.with_prefix, namespace_.is_glob, namespace_.symbols);
                StaticScope* scope = m_scope_resolver.currentScope();

                visit(*namespace_.ast, /* register_declarations= */ true);
                // dual visit so that we can handle forward references
                visit(*namespace_.ast, /* register_declarations= */ false);

                // if we had specific symbols to import, check that those exist
                if (!namespace_.symbols.empty())
                {
                    const auto it = std::ranges::find_if(
                        namespace_.symbols,
                        [&scope](const std::string& sym) -> bool {
                            return !scope->get(sym, true).has_value();
                        });

                    if (it != namespace_.symbols.end())
                        throw CodeError(
                            fmt::format("ImportError: Can not import symbol {} from {}, as it isn't in the package", *it, namespace_.name),
                            CodeErrorContext(
                                namespace_.ast->filename(),
                                namespace_.ast->position(),
                                "import"));
                }

                m_scope_resolver.saveNamespaceAndRemove();
                break;
            }

            default:
                break;
        }
    }

    void NameResolutionPass::visitKeyword(Node& node, const Keyword keyword, const bool register_declarations)
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
                    visit(node.list()[2], register_declarations);
                if (node.constList().size() > 1 && node.constList()[1].nodeType() == NodeType::Symbol)
                {
                    const std::string& name = node.constList()[1].string();
                    if (m_language_symbols.contains(name) && register_declarations)
                        throw CodeError(
                            fmt::format("Can not use a reserved identifier ('{}') as a {} name.", name, keyword == Keyword::Let ? "constant" : "variable"),
                            CodeErrorContext(
                                node.filename(),
                                node.constList()[1].position(),
                                name));

                    if (m_scope_resolver.isInScope(name) && keyword == Keyword::Let && register_declarations)
                        throw CodeError(
                            fmt::format("MutabilityError: Can not use 'let' to redefine variable `{}'", name),
                            CodeErrorContext(
                                node.filename(),
                                node.constList()[1].position(),
                                name));
                    if (keyword == Keyword::Set && m_scope_resolver.isRegistered(name))
                    {
                        if (m_scope_resolver.isImmutable(name).value_or(false) && register_declarations)
                            throw CodeError(
                                fmt::format("MutabilityError: Can not set the constant `{}' to {}", name, node.constList()[2].repr()),
                                CodeErrorContext(
                                    node.filename(),
                                    node.constList()[1].position(),
                                    name));

                        updateSymbolWithFullyQualifiedName(node.list()[1]);
                    }
                    else if (keyword != Keyword::Set)
                    {
                        // update the declared variable name to use the fully qualified name
                        // this will prevent name conflicts, and handle scope resolution
                        const std::string fully_qualified_name = addDefinedSymbol(name, keyword != Keyword::Let);
                        if (register_declarations)
                            node.list()[1].setString(fully_qualified_name);
                    }
                }
                break;

            case Keyword::Import:
                if (!node.constList().empty())
                    m_plugin_names.push_back(node.constList()[1].constList().back().string());
                break;

            case Keyword::While:
                // create a new scope to track variables
                m_scope_resolver.createNew();
                for (auto& child : node.list())
                    visit(child, register_declarations);
                // remove the scope once the loop has been compiled, only we were registering declarations
                m_scope_resolver.removeLastScope();
                break;

            case Keyword::Fun:
                // create a new scope to track variables
                m_scope_resolver.createNew();

                if (node.constList()[1].nodeType() == NodeType::List)
                {
                    for (auto& child : node.list()[1].list())
                    {
                        if (child.nodeType() == NodeType::Capture)
                        {
                            if (!m_scope_resolver.isRegistered(child.string()) && register_declarations)
                                throw CodeError(
                                    fmt::format("Can not capture `{}' because it is referencing a variable defined in an unreachable scope.", child.string()),
                                    CodeErrorContext(
                                        child.filename(),
                                        child.position(),
                                        child.repr()));

                            // save the old unqualified name of the capture, so that we can use it in the
                            // ASTLowerer later one
                            if (!child.getUnqualifiedName())
                            {
                                child.setUnqualifiedName(child.string());
                                m_defined_symbols.emplace(child.string());
                            }
                            // update the declared variable name to use the fully qualified name
                            // this will prevent name conflicts, and handle scope resolution
                            std::string old_name = child.string();
                            updateSymbolWithFullyQualifiedName(child);
                            // FIXME: addDefinedSymbol(fqn, true); ?
                            addDefinedSymbol(old_name, true);
                        }
                        else if (child.nodeType() == NodeType::Symbol)
                            addDefinedSymbol(child.string(), /* is_mutable= */ true);
                    }
                }
                if (node.constList().size() > 2)
                    visit(node.list()[2], register_declarations);

                // remove the scope once the function has been compiled, only we were registering declarations
                m_scope_resolver.removeLastScope();
                break;

            default:
                for (auto& child : node.list())
                    visit(child, register_declarations);
                break;
        }
    }

    void NameResolutionPass::addSymbolNode(const Node& symbol, const std::string& old_name)
    {
        const std::string& name = symbol.string();

        // we don't accept builtins/operators as a user symbol
        if (m_language_symbols.contains(name))
            return;

        // remove the old name node, to avoid false positive when looking for unbound symbols
        if (!old_name.empty())
        {
            auto it = std::ranges::find_if(m_symbol_nodes, [&old_name, &symbol](const Node& sym_node) -> bool {
                return sym_node.string() == old_name &&
                    sym_node.col() == symbol.col() &&
                    sym_node.line() == symbol.line() &&
                    sym_node.filename() == symbol.filename();
            });
            if (it != m_symbol_nodes.end())
            {
                it->setString(name);
                return;
            }
        }

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

    std::string NameResolutionPass::updateSymbolWithFullyQualifiedName(Node& symbol)
    {
        auto [allowed, fqn] = m_scope_resolver.canFullyQualifyName(symbol.string());

        if (m_language_symbols.contains(fqn) && symbol.string() != fqn)
        {
            throw CodeError(
                fmt::format(
                    "Symbol `{}' was resolved to `{}', which is also a builtin name. Either the symbol or the package it's in needs to be renamed to avoid conflicting with the builtin.",
                    symbol.string(), fqn),
                CodeErrorContext(
                    symbol.filename(),
                    symbol.position(),
                    symbol.repr()));
        }
        if (!allowed)
        {
            std::string message;
            if (fqn.ends_with("#hidden"))
                message = fmt::format(
                    R"(Unbound variable "{}". However, it exists in a namespace as "{}", did you forget to add it to the symbol list while importing?)",
                    symbol.string(),
                    fqn.substr(0, fqn.find_first_of('#')));
            else
                message = fmt::format(R"(Unbound variable "{}". However, it exists in a namespace as "{}", did you forget to prefix it with its namespace?)", symbol.string(), fqn);

            if (m_logger.shouldTrace())
                m_ast.debugPrint(std::cout) << '\n';

            throw CodeError(
                message,
                CodeErrorContext(
                    symbol.filename(),
                    symbol.position(),
                    symbol.repr()));
        }

        symbol.setString(fqn);
        return fqn;
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
                {
                    const std::string prefix = suggestion.substr(0, suggestion.find_first_of(':'));
                    const std::string note_about_prefix = fmt::format(
                        " You either forgot to import it in the symbol list (eg `(import {} :{})') or need to fully qualify it by adding the namespace",
                        prefix,
                        str);
                    const bool add_note = suggestion.ends_with(":" + str);
                    message = fmt::format(R"(Unbound variable error "{}" (did you mean "{}"?{}))", str, suggestion, add_note ? note_about_prefix : "");
                }

                throw CodeError(message, CodeErrorContext(sym.filename(), sym.position(), sym.repr()));
            }
        }
    }

    std::string NameResolutionPass::offerSuggestion(const std::string& str) const
    {
        auto iterate = [](const std::string& word, const std::unordered_set<std::string>& dict) -> std::string {
            std::string suggestion;
            // our suggestion shouldn't require more than half the string to change
            std::size_t suggestion_distance = word.size() / 2;
            for (const std::string& symbol : dict)
            {
                const std::size_t current_distance = Utils::levenshteinDistance(word, symbol);
                if (current_distance <= suggestion_distance)
                {
                    suggestion_distance = current_distance;
                    suggestion = symbol;
                }
            }
            return suggestion;
        };

        std::string suggestion = iterate(str, m_defined_symbols);
        // look for a suggestion related to language builtins
        if (suggestion.empty())
            suggestion = iterate(str, m_language_symbols);
        // look for a suggestion related to a namespace change
        if (suggestion.empty())
        {
            if (const auto it = std::ranges::find_if(m_defined_symbols, [&str](const std::string& symbol) {
                    return symbol.ends_with(":" + str);
                });
                it != m_defined_symbols.end())
                suggestion = *it;
        }

        return suggestion;
    }
}
