#include <Ark/Compiler/NameResolution/NameResolutionPass.hpp>

#include <Ark/Error/Exceptions.hpp>
#include <Ark/Utils/Utils.hpp>
#include <Ark/Builtins/Builtins.hpp>

#include <ranges>
#include <cassert>

#include <fmt/ranges.h>

namespace Ark::internal
{
    NameResolutionPass::NameResolutionPass(const unsigned debug, Statistics* stats_collector) :
        Pass("NameResolution", debug, stats_collector)
    {
        for (const auto& key : Builtins::builtins | std::views::keys)
            m_language_symbols.emplace(key);
        for (auto ope : Language::operators)
            m_language_symbols.emplace(ope);
        for (auto inst : Language::listInstructions)
            m_language_symbols.emplace(inst);

        m_language_symbols.emplace(Language::And);
        m_language_symbols.emplace(Language::Or);
        m_language_symbols.emplace(Language::SysArgs);
        m_language_symbols.emplace(Language::SysProgramName);
        m_language_symbols.emplace(Language::SysVersion);
        m_language_symbols.emplace(Language::Apply);

        m_scope_resolver.createNew();
    }

    void NameResolutionPass::setup(const std::vector<Import>& root_import_list)
    {
        m_logger.info("Configuring NameResolutionPass. Root is importing:");
        for (const Import& import : root_import_list)
        {
            if (import.is_glob)
                m_logger.info("  - {} :*", import.package);
            else if (import.with_prefix)
                m_logger.info("  - {}", import.package);
            else
                m_logger.info("  - {} :{}", import.package, fmt::join(import.symbols, " :"));
        }

        // the root namespace does NOT have a prefix!
        m_scope_resolver.createNewNamespace("", "", root_import_list);
    }

    void NameResolutionPass::process(const Node& ast)
    {
        assert(m_scope_resolver.currentScope()->isNamespace() && "The first scope should be a namespace");

        m_logger.traceStart("process");

        m_ast = ast;
        registerDeclarations(m_ast);
        qualifyNames(m_ast);

        addStat("NameResolution.process", m_logger.traceEnd());

        m_logger.debug("AST after name resolution");
        if (m_logger.shouldDebug())
            m_ast.debugPrint(std::cout) << '\n';

        m_logger.traceStart("checkForUndefinedSymbol");
        checkForUndefinedSymbol();
        addStat("NameResolution.checkForUndefinedSymbol", m_logger.traceEnd());
    }

    const Node& NameResolutionPass::ast() const noexcept
    {
        return m_ast;
    }

    std::string NameResolutionPass::qualifyAndRegisterSymbol(const std::string& sym, const bool is_mutable)
    {
        const std::string fully_qualified_name = m_scope_resolver.registerInCurrent(sym, is_mutable);
        m_defined_symbols.emplace(fully_qualified_name);
        return fully_qualified_name;
    }

    void NameResolutionPass::registerDeclarations(Node& node)
    {
        if (node.nodeType() == NodeType::List && !node.constList().empty())
        {
            if (node.constList()[0].nodeType() == NodeType::Keyword)
            {
                const Keyword keyword = node.constList()[0].keyword();

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
                            registerDeclarations(node.list()[2]);
                        if (node.constList().size() > 1 && node.constList()[1].nodeType() == NodeType::Symbol)
                        {
                            const std::string& name = node.constList()[1].string();
                            if (m_language_symbols.contains(name))
                                throw CodeError(
                                    fmt::format("Can not use a reserved identifier ('{}') as a {} name.", name, keyword == Keyword::Let ? "constant" : "variable"),
                                    CodeErrorContext(node.filename(), node.constList()[1].position()));

                            // todo: maybe we'll have errors here (in the diag tests) because names aren't fully qualified anymore in this first pass

                            if (m_scope_resolver.isInScope(name) && keyword == Keyword::Let)
                                throw CodeError(
                                    fmt::format("MutabilityError: Can not use 'let' to redefine variable `{}'", name),
                                    CodeErrorContext(node.filename(), node.constList()[1].position()));
                            if (keyword == Keyword::Set && m_scope_resolver.isRegistered(name) && m_scope_resolver.isImmutable(name).value_or(false))
                                throw CodeError(
                                    fmt::format("MutabilityError: Can not set the constant `{}' to {}", name, node.constList()[2].repr()),
                                    CodeErrorContext(node.filename(), node.constList()[1].position()));
                        }
                        break;

                    case Keyword::Import:
                        if (node.constList().size() >= 2 && !node.constList()[1].constList().empty())
                            m_plugin_names.push_back(node.constList()[1].constList().back().string());
                        break;

                    case Keyword::While:
                        // create a new scope to track variables
                        m_scope_resolver.createNew();
                        for (auto& child : node.list())
                            registerDeclarations(child);
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
                                    if (!m_scope_resolver.isRegistered(child.string()))
                                        throw CodeError(
                                            fmt::format("Can not capture `{}' because it is referencing a variable defined in an unreachable scope.", child.string()),
                                            CodeErrorContext(child.filename(), child.position()));

                                    // save the old unqualified name of the capture, so that we can use it in the
                                    // ASTLowerer later one
                                    if (!child.getUnqualifiedName())
                                    {
                                        child.setUnqualifiedName(child.string());
                                        m_defined_symbols.emplace(child.string());
                                    }
                                }
                                else if (child.nodeType() == NodeType::Symbol || child.nodeType() == NodeType::RefArg)
                                    m_defined_symbols.emplace(m_scope_resolver.registerInCurrent(child.string(), /* is_mutable= */ false));
                                else if (child.nodeType() == NodeType::MutArg)
                                    m_defined_symbols.emplace(m_scope_resolver.registerInCurrent(child.string(), /* is_mutable= */ true));
                            }
                        }
                        if (node.constList().size() > 2)
                            registerDeclarations(node.list()[2]);

                        // remove the scope once the function has been compiled, only we were registering declarations
                        m_scope_resolver.removeLastScope();
                        break;

                    case Keyword::Begin:
                        [[fallthrough]];
                    case Keyword::Del:
                        [[fallthrough]];
                    case Keyword::If:
                        for (auto& child : node.list())
                            registerDeclarations(child);
                        break;
                }
            }
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
                            CodeErrorContext(node.filename(), node.constList()[1].position()));

                    // check that we aren't doing a (append! a a) nor a (concat! a a)
                    if (funcname == Language::AppendInPlace || funcname == Language::ConcatInPlace)
                    {
                        for (std::size_t i = 2, end = node.constList().size(); i < end; ++i)
                        {
                            if (node.constList()[i].nodeType() == NodeType::Symbol && node.constList()[i].string() == arg)
                                throw CodeError(
                                    fmt::format("MutabilityError: Can not {} the list `{}' to itself", funcname, arg),
                                    CodeErrorContext(node.filename(), node.constList()[1].position()));
                        }
                    }
                }

                for (auto& child : node.list())
                    registerDeclarations(child);
            }
        }
        else if (node.nodeType() == NodeType::Namespace)
        {
            auto& namespace_ = node.arkNamespace();
            // we want to keep the namespace node (which will get ignored by the compiler,
            // that only uses its AST), so that we can (re)construct the scopes correctly
            m_scope_resolver.createNewNamespace(namespace_.name, namespace_.package, namespace_.imports);
            StaticScope* scope = m_scope_resolver.currentScope();

            registerDeclarations(*namespace_.ast);
            qualifyNames(*namespace_.ast);

            // if we had specific symbols to import, check that those exist
            if (!namespace_.symbols.empty())
            {
                const auto it = std::ranges::find_if(
                    namespace_.symbols,
                    [&scope, &namespace_](const std::string& sym) -> bool {
                        return !scope->get(sym, namespace_.name).has_value();
                    });

                if (it != namespace_.symbols.end())
                    throw CodeError(
                        fmt::format("ImportError: Can not import symbol {} from {}, as it isn't in the package", *it, namespace_.name),
                        CodeErrorContext(namespace_.ast->filename(), namespace_.ast->position()));
            }

            m_scope_resolver.saveNamespaceAndRemove();
        }
    }

    void NameResolutionPass::qualifyNames(Node& node)
    {
        switch (node.nodeType())
        {
            case NodeType::Symbol:
            {
                tryQualifyAndUpdateSymbol(node);
                savedAsKnownSymbol(node);
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
                        savedAsKnownSymbol(child);
                        statIncrementCount(Stats::FullyQualifiedNames);
                    }
                    else
                        savedAsKnownSymbol(child);
                }
                break;

            case NodeType::List:
                if (!node.constList().empty())
                {
                    if (node.constList()[0].nodeType() == NodeType::Keyword)
                    {
                        const Keyword keyword = node.constList()[0].keyword();

                        switch (keyword)
                        {
                            case Keyword::Set:
                                [[fallthrough]];
                            case Keyword::Let:
                                [[fallthrough]];
                            case Keyword::Mut:
                                // first, visit the value, then qualify the symbol
                                // this allows us to detect things like (let foo (fun (&foo) ()))
                                if (node.constList().size() > 2)
                                    qualifyNames(node.list()[2]);
                                if (node.constList().size() > 1 && node.constList()[1].nodeType() == NodeType::Symbol)
                                {
                                    const std::string& name = node.constList()[1].string();

                                    if (keyword == Keyword::Set && m_scope_resolver.isRegistered(name))
                                        tryQualifyAndUpdateSymbol(node.list()[1]);
                                    else if (keyword != Keyword::Set)
                                    {
                                        // update the declared variable name to use the fully qualified name
                                        // this will prevent name conflicts, and handle scope resolution
                                        const std::string fully_qualified_name = qualifyAndRegisterSymbol(name, keyword != Keyword::Let);
                                        node.list()[1].setString(fully_qualified_name);
                                    }
                                }
                                break;

                            case Keyword::Import:
                                break;

                            case Keyword::While:
                                m_scope_resolver.createNew();
                                for (auto& child : node.list())
                                    qualifyNames(child);
                                m_scope_resolver.removeLastScope();
                                break;

                            case Keyword::Fun:
                                m_scope_resolver.createNew();

                                if (node.constList()[1].nodeType() == NodeType::List)
                                {
                                    for (auto& child : node.list()[1].list())
                                    {
                                        if (child.nodeType() == NodeType::Capture)
                                        {
                                            if (!m_scope_resolver.isRegistered(child.string()))
                                                throw CodeError(
                                                    fmt::format("Can not capture `{}' because it is referencing a variable defined in an unreachable scope.", child.string()),
                                                    CodeErrorContext(child.filename(), child.position()));

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
                                            tryQualifyAndUpdateSymbol(child);
                                            qualifyAndRegisterSymbol(old_name, true);  // todo: why do we use old name? -> because we qualified previously...
                                            // todo: we could just register the symbol ourselves here
                                        }
                                        else if (child.nodeType() == NodeType::Symbol || child.nodeType() == NodeType::RefArg)
                                            qualifyAndRegisterSymbol(child.string(), /* is_mutable= */ false);
                                        else if (child.nodeType() == NodeType::MutArg)
                                            qualifyAndRegisterSymbol(child.string(), /* is_mutable= */ true);
                                    }
                                }
                                if (node.constList().size() > 2)
                                    qualifyNames(node.list()[2]);

                                m_scope_resolver.removeLastScope();
                                break;

                            default:
                                for (auto& child : node.list())
                                    qualifyNames(child);
                                break;
                        }
                    }
                    else
                    {

                        for (auto& child : node.list())
                            qualifyNames(child);
                    }
                }
                break;

            case NodeType::Namespace:
                [[fallthrough]];
            case NodeType::MutArg:
                [[fallthrough]];
            case NodeType::RefArg:
                [[fallthrough]];
            case NodeType::Capture:
                [[fallthrough]];
            case NodeType::Keyword:
                [[fallthrough]];
            case NodeType::String:
                [[fallthrough]];
            case NodeType::Number:
                [[fallthrough]];
            case NodeType::Spread:
                [[fallthrough]];
            case NodeType::Macro:
                [[fallthrough]];
            case NodeType::Unused:
                break;
        }
    }

    void NameResolutionPass::savedAsKnownSymbol(const Node& symbol)
    {
        const std::string& name = symbol.string();

        // we don't accept builtins/operators as a user symbol
        if (m_language_symbols.contains(name))
            return;

        // todo: maybe we could get rid of the check
        const auto it = std::ranges::find_if(
            m_symbol_nodes,
            [&name](const Node& sym_node) -> bool {
                return sym_node.string() == name;
            });
        if (it == m_symbol_nodes.end())
            m_symbol_nodes.push_back(symbol);
    }

    bool NameResolutionPass::mayBeFromPlugin(const std::string& name) const noexcept
    {
        const std::string prefix = name.substr(0, name.find_first_of(':'));
        return std::ranges::find(m_plugin_names, prefix) != m_plugin_names.end();
    }

    std::string NameResolutionPass::tryQualifyAndUpdateSymbol(Node& symbol)
    {
        if (m_language_symbols.contains(symbol.string()))
            return symbol.string();

        auto [maybe_error, fqn] = m_scope_resolver.tryFullyQualifyName(symbol.string());

        if (m_language_symbols.contains(fqn) && symbol.string() != fqn)
        {
            throw CodeError(
                fmt::format(
                    "Symbol `{}' was resolved to `{}', which is also a builtin name. Either the symbol or the package it's in needs to be renamed to avoid conflicting with the builtin.",
                    symbol.string(), fqn),
                CodeErrorContext(symbol.filename(), symbol.position()));
        }

        if (maybe_error.has_value())
        {
            std::string message;
            switch (maybe_error.value())
            {
                case QualifyingFailReason::NotInSymbolList:
                    message = fmt::format(
                        R"(Unbound variable "{}". However, it exists in a namespace as "{}", did you forget to add it to the symbol list while importing?)",
                        symbol.string(),
                        fqn);
                    break;

                case QualifyingFailReason::NeedPrefixing:
                    message = fmt::format(R"(Unbound variable "{}". However, it exists in a namespace as "{}", did you forget to prefix it with its namespace?)", symbol.string(), fqn);
                    break;
            }

            if (m_logger.shouldTrace())
                m_ast.debugPrint(std::cout) << '\n';

            throw CodeError(message, CodeErrorContext(symbol.filename(), symbol.position()));
        }

        symbol.setString(fqn);
        statIncrementCount(Stats::FullyQualifiedNames);
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
                    message = fmt::format(R"(Unbound variable "{}" (variable is used but not defined))", str);
                else if (false)  // todo hidden / unreachable
                {
                    const std::string prefix = suggestion.substr(0, suggestion.find_first_of(':'));
                    const std::string suffix = suggestion.substr(suggestion.find_first_of(':') + 1, suggestion.size() - prefix.size() - 1);
                    message = fmt::format(R"(Unbound variable "{0}". Did you forget to add '{1}' when importing '{2}', eg `(import {2} :{1})'? (symbol is visible but not available))", str, suffix, prefix);
                }
                else
                {
                    const std::string prefix = suggestion.substr(0, suggestion.find_first_of(':'));
                    const std::string note_about_prefix = fmt::format(
                        " You either forgot to import it in the symbol list (eg `(import {} :{})') or need to fully qualify it by adding the namespace",
                        prefix,
                        str);
                    const bool add_note = suggestion.ends_with(":" + str);
                    message = fmt::format(R"(Unbound variable "{}" (did you mean "{}"?{}))", str, suggestion, add_note ? note_about_prefix : "");
                }

                throw CodeError(message, CodeErrorContext(sym.filename(), sym.position()));
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
