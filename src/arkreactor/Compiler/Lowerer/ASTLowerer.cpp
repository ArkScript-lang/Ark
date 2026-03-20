#include <Ark/Compiler/Lowerer/ASTLowerer.hpp>

#include <cassert>
#include <ranges>
#include <utility>
#include <algorithm>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <Ark/Error/Exceptions.hpp>
#include <Ark/Error/Diagnostics.hpp>
#include <Ark/Utils/Literals.hpp>
#include <Ark/Builtins/Builtins.hpp>

namespace Ark::internal
{
    using namespace literals;

    enum class CallType
    {
        Classic,
        SelfNotRecursive,
        Symbol,
        SymbolByIndex,
        Builtin
    };

    ASTLowerer::ASTLowerer(const unsigned debug) :
        Pass("ASTLowerer", debug)
    {}

    void ASTLowerer::addToTables(const std::vector<std::string>& symbols, const std::vector<ValTableElem>& constants)
    {
        std::ranges::copy(symbols, std::back_inserter(m_symbols));
        std::ranges::copy(constants, std::back_inserter(m_values));
    }

    void ASTLowerer::offsetPagesBy(const std::size_t offset)
    {
        m_start_page_at_offset = offset;
    }

    void ASTLowerer::process(Node& ast)
    {
        m_logger.traceStart("process");
        const Page global = createNewCodePage();

        // gather symbols, values, and start to create code segments
        compileExpression(
            ast,
            /* current_page */ global,
            /* is_result_unused= */ true,
            /* is_terminal= */ false);
        m_logger.traceEnd();
    }

    const std::vector<IR::Block>& ASTLowerer::intermediateRepresentation() const noexcept
    {
        return m_code_pages;
    }

    const std::vector<std::string>& ASTLowerer::symbols() const noexcept
    {
        return m_symbols;
    }

    const std::vector<ValTableElem>& ASTLowerer::values() const noexcept
    {
        return m_values;
    }

    std::optional<Instruction> ASTLowerer::getOperator(const std::string& name) noexcept
    {
        const auto it = std::ranges::find(Language::operators, name);
        if (it != Language::operators.end())
            return static_cast<Instruction>(std::distance(Language::operators.begin(), it) + FIRST_OPERATOR);
        return std::nullopt;
    }

    std::optional<uint16_t> ASTLowerer::getBuiltin(const std::string& name) noexcept
    {
        const auto it = std::ranges::find_if(Builtins::builtins,
                                             [&name](const std::pair<std::string, Value>& element) -> bool {
                                                 return name == element.first;
                                             });
        if (it != Builtins::builtins.end())
            return static_cast<uint16_t>(std::distance(Builtins::builtins.begin(), it));
        return std::nullopt;
    }

    std::optional<Instruction> ASTLowerer::getListInstruction(const std::string& name) noexcept
    {
        const auto it = std::ranges::find(Language::listInstructions, name);
        if (it != Language::listInstructions.end())
            return static_cast<Instruction>(std::distance(Language::listInstructions.begin(), it) + LIST);
        return std::nullopt;
    }

    bool ASTLowerer::isBreakpoint(const Node& node)
    {
        if (node.nodeType() == NodeType::List && !node.constList().empty() && node.constList()[0].nodeType() == NodeType::Symbol)
            return node.constList().front().string() == "breakpoint";
        return false;
    }

    bool ASTLowerer::nodeProducesOutput(const Node& node)
    {
        if (node.nodeType() == NodeType::List && !node.constList().empty() && node.constList()[0].nodeType() == NodeType::Keyword)
            // a 'begin' node produces a value if the last node in it produces a value
            return (node.constList()[0].keyword() == Keyword::Begin && node.constList().size() > 1 && nodeProducesOutput(node.constList().back())) ||
                // a function always produces a value ; even if it ends with a node not producing one, the VM returns nil
                node.constList()[0].keyword() == Keyword::Fun ||
                // a let/mut/set pushes the value that was assigned
                node.constList()[0].keyword() == Keyword::Let ||
                node.constList()[0].keyword() == Keyword::Mut ||
                node.constList()[0].keyword() == Keyword::Set ||
                // a condition produces a value if all its branches produce a value
                (node.constList()[0].keyword() == Keyword::If &&
                 nodeProducesOutput(node.constList()[2]) &&
                 (node.constList().size() == 3 || nodeProducesOutput(node.constList()[3])));
        // breakpoint do not produce values
        if (node.nodeType() == NodeType::List && !node.constList().empty() && node.constList()[0].nodeType() == NodeType::Symbol)
        {
            const std::string& name = node.constList().front().string();
            return name != "breakpoint";
        }
        return true;  // any other node, function call, symbol, number...
    }

    bool ASTLowerer::isUnaryInst(const Instruction inst) noexcept
    {
        switch (inst)
        {
            case NOT: [[fallthrough]];
            case LEN: [[fallthrough]];
            case IS_EMPTY: [[fallthrough]];
            case TAIL: [[fallthrough]];
            case HEAD: [[fallthrough]];
            case IS_NIL: [[fallthrough]];
            case TO_NUM: [[fallthrough]];
            case TO_STR: [[fallthrough]];
            case TYPE:
                return true;

            default:
                return false;
        }
    }

    bool ASTLowerer::isTernaryInst(const Instruction inst) noexcept
    {
        switch (inst)
        {
            case AT_AT:
                return true;

            default:
                return false;
        }
    }

    bool ASTLowerer::isRepeatableOperation(const Instruction inst) noexcept
    {
        switch (inst)
        {
            case ADD: [[fallthrough]];
            case SUB: [[fallthrough]];
            case MUL: [[fallthrough]];
            case DIV:
                return true;

            default:
                return false;
        }
    }

    void ASTLowerer::warning(const std::string& message, const Node& node)
    {
        m_logger.warn("{}", Diagnostics::makeContextWithNode(message, node, m_logger.colorize()));
    }

    void ASTLowerer::buildAndThrowError(const std::string& message, const Node& node)
    {
        throw CodeError(message, CodeErrorContext(node.filename(), node.position()));
    }

    void ASTLowerer::makeError(const ErrorKind kind, const Node& node, const std::string& additional_ctx)
    {
        const std::string invalid_node_msg = "The given node doesn't return a value, and thus can't be used as an expression.";

        switch (kind)
        {
            case ErrorKind::InvalidNodeMacro:
                buildAndThrowError(fmt::format("Invalid node ; if it was computed by a macro, check that a node is returned"), node);
                break;

            case ErrorKind::InvalidNodeNoReturnValue:
                buildAndThrowError(fmt::format("Invalid node inside call to `{}'. {}", additional_ctx, invalid_node_msg), node);
                break;

            case ErrorKind::InvalidNodeInTailCallNoReturnValue:
                buildAndThrowError(fmt::format("Invalid node inside tail call to `{}'. {}", additional_ctx, invalid_node_msg), node);
                break;

            case ErrorKind::InvalidNodeInOperatorNoReturnValue:
                buildAndThrowError(fmt::format("Invalid node inside call to operator `{}'. {}", additional_ctx, invalid_node_msg), node);
                break;
        }
    }

    void ASTLowerer::compileExpression(Node& x, const Page p, const bool is_result_unused, const bool is_terminal)
    {
        // register symbols
        if (x.nodeType() == NodeType::Symbol)
            compileSymbol(x, p, is_result_unused, /* can_use_ref= */ true);
        else if (x.nodeType() == NodeType::Field)
        {
            // the parser guarantees us that there is at least 2 elements (eg: a.b)
            compileSymbol(x.list()[0], p, is_result_unused, /* can_use_ref= */ true);
            for (auto it = x.constList().begin() + 1, end = x.constList().end(); it != end; ++it)
            {
                uint16_t i = addSymbol(*it);
                page(p).emplace_back(GET_FIELD, i);
            }
            page(p).back().setSourceLocation(x.filename(), x.position().start.line);
        }
        // register values
        else if (x.nodeType() == NodeType::String || x.nodeType() == NodeType::Number)
        {
            uint16_t i = addValue(x);

            if (!is_result_unused)
                page(p).emplace_back(LOAD_CONST, i);
        }
        // namespace nodes
        else if (x.nodeType() == NodeType::Namespace)
            compileExpression(*x.constArkNamespace().ast, p, is_result_unused, is_terminal);
        else if (x.nodeType() == NodeType::List)
        {
            // empty code block should be nil
            if (x.constList().empty())
            {
                if (!is_result_unused)
                {
                    static const std::optional<uint16_t> nil = getBuiltin("nil");
                    page(p).emplace_back(BUILTIN, nil.value());
                }
            }
            // list instructions
            else if (const auto head = x.constList()[0]; head.nodeType() == NodeType::Symbol && getListInstruction(head.string()).has_value())
                compileListInstruction(x, p, is_result_unused);
            else if (head.nodeType() == NodeType::Symbol && head.string() == Language::Apply)
                compileApplyInstruction(x, p, is_result_unused);
            // registering structures
            else if (head.nodeType() == NodeType::Keyword)
            {
                switch (const Keyword keyword = head.keyword())
                {
                    case Keyword::If:
                        compileIf(x, p, is_result_unused, is_terminal);
                        break;

                    case Keyword::Set:
                        [[fallthrough]];
                    case Keyword::Let:
                        [[fallthrough]];
                    case Keyword::Mut:
                        compileLetMutSet(keyword, x, p, is_result_unused);
                        break;

                    case Keyword::Fun:
                        compileFunction(x, p, is_result_unused);
                        break;

                    case Keyword::Begin:
                    {
                        for (std::size_t i = 1, size = x.list().size(); i < size; ++i)
                            compileExpression(
                                x.list()[i],
                                p,
                                // All the nodes in a 'begin' (except for the last one) are producing a result that we want to drop.
                                /* is_result_unused= */ (i != size - 1) || is_result_unused,
                                // If the 'begin' is a terminal node, only its last node is terminal.
                                /* is_terminal= */ is_terminal && (i == size - 1));
                        break;
                    }

                    case Keyword::While:
                        compileWhile(x, p);
                        break;

                    case Keyword::Import:
                        compilePluginImport(x, p);
                        break;

                    case Keyword::Del:
                        page(p).emplace_back(DEL, addSymbol(x.constList()[1]));
                        page(p).back().setSourceLocation(x.filename(), x.position().start.line);
                        break;
                }
            }
            else
            {
                // If we are here, we should have a function name via the m_opened_vars.
                // Push arguments first, then function name, then call it.
                handleCalls(x, p, is_result_unused, is_terminal);
            }
        }
        else if (x.nodeType() != NodeType::Unused)
            buildAndThrowError(
                fmt::format(
                    "NodeType `{}' not handled in ASTLowerer::compileExpression. Please fill an issue on GitHub: https://github.com/ArkScript-lang/Ark",
                    typeToString(x)),
                x);
    }

    void ASTLowerer::compileSymbol(const Node& x, const Page p, const bool is_result_unused, const bool can_use_ref)
    {
        const std::string& name = x.string();

        if (const auto it_builtin = getBuiltin(name))
            page(p).emplace_back(Instruction::BUILTIN, it_builtin.value());
        else if (getOperator(name).has_value())
            buildAndThrowError(fmt::format("Found a freestanding operator: `{}`. It can not be used as value like `+', where (let add +) (add 1 2) would be valid", name), x);
        else
        {
            if (can_use_ref)
            {
                const std::optional<std::size_t> maybe_local_idx = m_locals_locator.lookupLastScopeByName(name);
                if (maybe_local_idx.has_value())
                    page(p).emplace_back(LOAD_FAST_BY_INDEX, static_cast<uint16_t>(maybe_local_idx.value()));
                else
                    page(p).emplace_back(LOAD_FAST, addSymbol(x));
            }
            else
                page(p).emplace_back(LOAD_SYMBOL, addSymbol(x));
        }

        page(p).back().setSourceLocation(x.filename(), x.position().start.line);

        if (is_result_unused)
        {
            warning("Statement has no effect", x);
            page(p).emplace_back(POP);
            page(p).back().setSourceLocation(x.filename(), x.position().start.line);
        }
    }

    void ASTLowerer::compileListInstruction(Node& x, const Page p, const bool is_result_unused)
    {
        const Node head = x.constList()[0];
        const std::string& name = head.string();
        const Instruction inst = getListInstruction(name).value();

        // length of at least 1 since we got a symbol name
        const auto argc = x.constList().size() - 1u;
        // error, can not use append/concat/pop (and their in place versions) with a <2 length argument list
        if (argc < 2 && APPEND <= inst && inst <= SET_AT_2_INDEX)
            buildAndThrowError(fmt::format("Can not use {} with less than 2 arguments", name), head);
        if (std::cmp_greater(argc, MaxValue16Bits))
            buildAndThrowError(fmt::format("Too many arguments ({}), exceeds {}", argc, MaxValue16Bits), x);
        if (argc != 3 && inst == SET_AT_INDEX)
            buildAndThrowError(fmt::format("Expected 3 arguments (list, index, value) for {}, got {}", name, argc), head);
        if (argc != 4 && inst == SET_AT_2_INDEX)
            buildAndThrowError(fmt::format("Expected 4 arguments (list, y, x, value) for {}, got {}", name, argc), head);

        // compile arguments in reverse order
        for (std::size_t i = x.constList().size() - 1u; i > 0; --i)
        {
            Node& node = x.list()[i];
            if (nodeProducesOutput(node))
                compileExpression(node, p, false, false);
            else
                makeError(ErrorKind::InvalidNodeNoReturnValue, node, name);
        }

        // put inst and number of arguments
        std::size_t inst_argc = 0;
        switch (inst)
        {
            case LIST:
                inst_argc = argc;
                break;

            case APPEND:
                [[fallthrough]];
            case APPEND_IN_PLACE:
                [[fallthrough]];
            case CONCAT:
                [[fallthrough]];
            case CONCAT_IN_PLACE:
                inst_argc = argc - 1;
                break;

            case POP_LIST:
                inst_argc = 0;
                break;

            case SET_AT_INDEX:
                [[fallthrough]];
            case SET_AT_2_INDEX:
                [[fallthrough]];
            case POP_LIST_IN_PLACE:
                inst_argc = is_result_unused ? 0 : 1;
                break;

            default:
                break;
        }
        page(p).emplace_back(inst, static_cast<uint16_t>(inst_argc));
        page(p).back().setSourceLocation(head.filename(), head.position().start.line);

        if (!is_result_unused && (inst == APPEND_IN_PLACE || inst == CONCAT_IN_PLACE))
        {
            // Load the first argument which should be a symbol (or field),
            // that append!/concat! write to, so that we have its new value available.
            compileExpression(x.list()[1], p, false, false);
        }

        // append!, concat!, pop!, @= and @@= can push to the stack, but not using its returned value isn't an error
        if (is_result_unused && (inst == LIST || inst == APPEND || inst == CONCAT || inst == POP_LIST))
        {
            warning("Ignoring return value of function", x);
            page(p).emplace_back(POP);
        }
    }

    void ASTLowerer::compileApplyInstruction(Node& x, const Page p, const bool is_result_unused)
    {
        const Node head = x.constList()[0];
        const auto argc = x.constList().size() - 1u;

        if (argc != 2)
            buildAndThrowError(fmt::format("Expected 2 arguments (function, arguments) for apply, got {}", argc), head);

        const auto label_return = IR::Entity::Label(m_current_label++);
        page(p).emplace_back(IR::Entity::Goto(label_return, PUSH_RETURN_ADDRESS));

        for (Node& node : x.list() | std::ranges::views::drop(1))
        {
            if (nodeProducesOutput(node))
                compileExpression(node, p, false, false);
            else
                makeError(ErrorKind::InvalidNodeNoReturnValue, node, "apply");
        }
        page(p).emplace_back(APPLY);
        // patch the PUSH_RETURN_ADDRESS instruction with the return location (IP=CALL instruction IP)
        page(p).emplace_back(label_return);

        if (is_result_unused)
            page(p).emplace_back(POP);
    }

    void ASTLowerer::compileIf(Node& x, const Page p, const bool is_result_unused, const bool is_terminal)
    {
        if (x.constList().size() == 1)
            buildAndThrowError("Invalid condition: missing 'cond' and 'then' nodes, expected (if cond then)", x);
        if (x.constList().size() == 2)
            buildAndThrowError(fmt::format("Invalid condition: missing 'then' node, expected (if {} then)", x.constList()[1].repr()), x);

        // compile condition
        compileExpression(x.list()[1], p, false, false);
        page(p).back().setSourceLocation(x.constList()[1].filename(), x.constList()[1].position().start.line);

        // jump only if needed to the "true" branch
        const auto label_then = IR::Entity::Label(m_current_label++);
        page(p).emplace_back(IR::Entity::GotoIf(label_then, true));

        // "false" branch code
        if (x.constList().size() == 4)  // we have an else clause
        {
            m_locals_locator.saveScopeLengthForBranch();
            compileExpression(x.list()[3], p, is_result_unused, is_terminal);
            page(p).back().setSourceLocation(x.constList()[3].filename(), x.constList()[3].position().start.line);
            m_locals_locator.dropVarsForBranch();
        }

        // when else is finished, jump to end
        const auto label_end = IR::Entity::Label(m_current_label++);
        page(p).emplace_back(IR::Entity::Goto(label_end));

        // absolute address to jump to if condition is true
        page(p).emplace_back(label_then);
        // if code
        m_locals_locator.saveScopeLengthForBranch();
        compileExpression(x.list()[2], p, is_result_unused, is_terminal);
        page(p).back().setSourceLocation(x.constList()[2].filename(), x.constList()[2].position().start.line);
        m_locals_locator.dropVarsForBranch();
        // set jump to end pos
        page(p).emplace_back(label_end);
    }

    void ASTLowerer::compileFunction(Node& x, const Page p, const bool is_result_unused)
    {
        if (const auto args = x.constList()[1]; args.nodeType() != NodeType::List)
            buildAndThrowError(fmt::format("Expected a well formed argument(s) list, got a {}", typeToString(args)), args);
        if (x.constList().size() != 3)
            makeError(ErrorKind::InvalidNodeMacro, x, "");

        // capture, if needed
        std::size_t capture_inst_count = 0;
        for (const auto& node : x.constList()[1].constList())
        {
            if (node.nodeType() == NodeType::Capture)
            {
                const uint16_t symbol_id = addSymbol(node);

                // We have an unqualified name that isn't the captured name
                // This means we need to rename the captured value
                if (const auto& maybe_nqn = node.getUnqualifiedName(); maybe_nqn.has_value() && maybe_nqn.value() != node.string())
                {
                    const uint16_t nqn_id = addSymbol(Node(NodeType::Symbol, maybe_nqn.value()));

                    page(p).emplace_back(RENAME_NEXT_CAPTURE, nqn_id);
                    page(p).emplace_back(CAPTURE, symbol_id);
                }
                else
                    page(p).emplace_back(CAPTURE, symbol_id);

                ++capture_inst_count;
            }
        }
        const bool is_closure = capture_inst_count > 0;

        m_locals_locator.createScope(
            is_closure
                ? LocalsLocator::ScopeType::Closure
                : LocalsLocator::ScopeType::Function);

        // create new page for function body
        const auto function_body_page = createNewCodePage();
        // save page_id into the constants table as PageAddr and load the const
        page(p).emplace_back(is_closure ? MAKE_CLOSURE : LOAD_CONST, addValue(function_body_page.index, x));

        std::size_t arg_count = 0;
        // pushing arguments from the stack into variables in the new scope
        for (const auto& node : x.constList()[1].constList() | std::ranges::views::reverse)
        {
            if (node.nodeType() == NodeType::Symbol || node.nodeType() == NodeType::MutArg)
            {
                page(function_body_page).emplace_back(STORE, addSymbol(node));
                m_locals_locator.addLocal(node.string());
                arg_count++;
            }
            else if (node.nodeType() == NodeType::RefArg)
            {
                page(function_body_page).emplace_back(STORE_REF, addSymbol(node));
                m_locals_locator.addLocal(node.string());
                arg_count++;
            }
        }

        // Register an opened variable as "#anonymous", which won't match any valid names inside ASTLowerer::handleCalls.
        // This way we can continue to safely apply optimisations on
        // (let name (fun (e) (map lst (fun (e) (name e)))))
        // Otherwise, `name` would have been optimized to a CALL_CURRENT_PAGE, which would have returned the wrong page.
        if (x.isAnonymousFunction())
            m_opened_vars.emplace("#anonymous", arg_count);
        // push body of the function
        compileExpression(x.list()[2], function_body_page, false, true);
        if (x.isAnonymousFunction())
            m_opened_vars.pop();

        // return last value on the stack
        page(function_body_page).emplace_back(RET);
        m_locals_locator.deleteScope();

        // if the computed function is unused, pop it
        if (is_result_unused)
        {
            warning("Unused declared function", x);
            page(p).emplace_back(POP);
        }
    }

    void ASTLowerer::compileLetMutSet(const Keyword n, Node& x, const Page p, const bool is_result_unused)
    {
        if (const auto sym = x.constList()[1]; sym.nodeType() != NodeType::Symbol)
            buildAndThrowError(fmt::format("Expected a symbol, got a {}", typeToString(sym)), sym);
        if (x.constList().size() != 3)
            makeError(ErrorKind::InvalidNodeMacro, x, "");

        const std::string name = x.constList()[1].string();
        uint16_t i = addSymbol(x.constList()[1]);

        if (!m_opened_vars.empty() && m_opened_vars.top().name == name)
            buildAndThrowError("Can not define a variable using the same name as the function it is defined inside. You need to rename the function or the variable", x);

        const bool is_function = x.constList()[2].isFunction();
        if (is_function)
        {
            std::size_t arg_count = 0;
            if (x.constList()[2].nodeType() == NodeType::List && x.constList()[2].constList().size() >= 2 &&
                x.constList()[2].constList()[1].nodeType() == NodeType::List)
            {
                for (const auto& node : x.constList()[2].constList()[1].constList())
                {
                    if (node.nodeType() == NodeType::Symbol || node.nodeType() == NodeType::MutArg || node.nodeType() == NodeType::RefArg)
                        arg_count++;
                }
            }
            m_opened_vars.push(Var(name, arg_count));
            x.list()[2].setFunctionKind(/* anonymous= */ false);
        }

        // put value before symbol id
        // starting at index = 2 because x is a (let|mut|set variable ...) node
        compileExpression(x.list()[2], p, false, false);

        if (n == Keyword::Let || n == Keyword::Mut)
        {
            page(p).emplace_back(STORE, i);
            m_locals_locator.addLocal(name);
        }
        else
            page(p).emplace_back(SET_VAL, i);

        if (!is_result_unused)
            page(p).emplace_back(LOAD_SYMBOL, i);

        if (is_function)
            m_opened_vars.pop();
        page(p).back().setSourceLocation(x.filename(), x.position().start.line);
    }

    void ASTLowerer::compileWhile(Node& x, const Page p)
    {
        if (x.constList().size() != 3)
            makeError(ErrorKind::InvalidNodeMacro, x, "");

        m_locals_locator.createScope();
        page(p).emplace_back(CREATE_SCOPE);
        page(p).back().setSourceLocation(x.filename(), x.position().start.line);

        // save current position to jump there at the end of the loop
        const auto label_loop = IR::Entity::Label(m_current_label++);
        page(p).emplace_back(label_loop);
        // push condition
        compileExpression(x.list()[1], p, false, false);
        // absolute jump to end of block if condition is false
        const auto label_end = IR::Entity::Label(m_current_label++);
        page(p).emplace_back(IR::Entity::GotoIf(label_end, false));
        // push code to page
        compileExpression(x.list()[2], p, true, false);

        // reset the scope at the end of the loop so that indices are still valid
        // otherwise, (while true { (let a 5) (print a) (let b 6) (print b) })
        // would print 5, 6, then only 6 as we emit LOAD_SYMBOL_FROM_INDEX 0 and b is the last in the scope
        // loop, jump to the condition
        page(p).emplace_back(IR::Entity::Goto(label_loop, RESET_SCOPE_JUMP));

        // absolute address to jump to if condition is false
        page(p).emplace_back(label_end);

        page(p).emplace_back(POP_SCOPE);
        m_locals_locator.deleteScope();
    }

    void ASTLowerer::compilePluginImport(const Node& x, const Page p)
    {
        std::string path;
        const Node package_node = x.constList()[1];
        for (std::size_t i = 0, end = package_node.constList().size(); i < end; ++i)
        {
            path += package_node.constList()[i].string();
            if (i + 1 != end)
                path += "/";
        }
        path += ".arkm";

        // register plugin path in the constants table
        uint16_t id = addValue(Node(NodeType::String, path));
        // add plugin instruction + id of the constant referring to the plugin path
        page(p).emplace_back(PLUGIN, id);
        page(p).back().setSourceLocation(x.filename(), x.position().start.line);
    }

    void ASTLowerer::pushFunctionCallArguments(Node& call, const Page p, const bool is_tail_call)
    {
        const auto node = call.constList()[0];

        for (Node& value : std::ranges::drop_view(call.list(), 1))
        {
            if (nodeProducesOutput(value) || isBreakpoint(value))
            {
                // we have to disallow usage of references in tail calls, because if we shuffle arguments around while using refs, they will end up with the same value
                if (value.nodeType() == NodeType::Symbol && is_tail_call)
                    compileSymbol(value, p, /* is_result_unused= */ false, /* can_use_ref= */ false);
                else
                    compileExpression(value, p, /* is_result_unused= */ false, /* is_terminal= */ false);
            }
            else
                makeError(is_tail_call ? ErrorKind::InvalidNodeInTailCallNoReturnValue : ErrorKind::InvalidNodeNoReturnValue, value, node.repr());
        }
    }

    void ASTLowerer::handleCalls(Node& x, const Page p, bool is_result_unused, const bool is_terminal)
    {
        const Node& node = x.constList()[0];
        bool matched = false;

        if (node.nodeType() == NodeType::Symbol)
        {
            if (node.string() == Language::And || node.string() == Language::Or)
            {
                matched = true;
                handleShortcircuit(x, p);
            }
            if (const auto maybe_operator = getOperator(node.string()); maybe_operator.has_value())
            {
                matched = true;
                if (maybe_operator.value() == BREAKPOINT)
                    is_result_unused = false;
                handleOperator(x, p, maybe_operator.value());
            }
        }

        if (!matched)
        {
            // if nothing else matched, then compile a function call
            if (handleFunctionCall(x, p, is_terminal))
                // if it returned true, we compiled a tail call, skip the POP at the end
                return;
        }

        if (is_result_unused)
            page(p).emplace_back(POP);
    }

    void ASTLowerer::handleShortcircuit(Node& x, const Page p)
    {
        const Node& node = x.constList()[0];
        const auto name = node.string();  // and / or
        const Instruction inst = name == Language::And ? SHORTCIRCUIT_AND : SHORTCIRCUIT_OR;

        // short circuit implementation
        if (x.constList().size() < 3)
            buildAndThrowError(
                fmt::format(
                    "Expected at least 2 arguments while compiling '{}', got {}",
                    name,
                    x.constList().size() - 1),
                x);

        if (!nodeProducesOutput(x.list()[1]))
            buildAndThrowError(
                fmt::format(
                    "Can not use `{}' inside a `{}' expression, as it doesn't return a value",
                    x.list()[1].repr(), name),
                x.list()[1]);
        compileExpression(x.list()[1], p, false, false);

        const auto label_shortcircuit = IR::Entity::Label(m_current_label++);
        auto shortcircuit_entity = IR::Entity::Goto(label_shortcircuit, inst);
        page(p).emplace_back(shortcircuit_entity);

        for (std::size_t i = 2, end = x.constList().size(); i < end; ++i)
        {
            if (!nodeProducesOutput(x.list()[i]))
                buildAndThrowError(
                    fmt::format(
                        "Can not use `{}' inside a `{}' expression, as it doesn't return a value",
                        x.list()[i].repr(), name),
                    x.list()[i]);
            compileExpression(x.list()[i], p, false, false);
            if (i + 1 != end)
                page(p).emplace_back(shortcircuit_entity);
        }

        page(p).emplace_back(label_shortcircuit);
    }

    void ASTLowerer::handleOperator(Node& x, const Page p, const Instruction op)
    {
        constexpr std::size_t start_index = 1;
        const Node& node = x.constList()[0];
        const auto op_name = Language::operators[static_cast<std::size_t>(op - FIRST_OPERATOR)];


        // push arguments on current page
        std::size_t exp_count = 0;
        for (std::size_t index = start_index, size = x.constList().size(); index < size; ++index)
        {
            const bool is_breakpoint = isBreakpoint(x.constList()[index]);
            if (nodeProducesOutput(x.constList()[index]) || is_breakpoint)
                compileExpression(x.list()[index], p, false, false);
            else
                makeError(ErrorKind::InvalidNodeInOperatorNoReturnValue, x.constList()[index], node.repr());

            if (!is_breakpoint)
                exp_count++;

            // in order to be able to handle things like (op A B C D...)
            // which should be transformed into A B op C op D op...
            if (exp_count >= 2 && !isTernaryInst(op) && !is_breakpoint)
                page(p).emplace_back(op);
        }

        if (isBreakpoint(x))
        {
            if (exp_count > 1)
                buildAndThrowError(fmt::format("`{}' expected at most one argument, but was called with {}", op_name, exp_count), x.constList()[0]);
            page(p).emplace_back(op, exp_count);
        }
        else if (isUnaryInst(op))
        {
            if (exp_count != 1)
                buildAndThrowError(fmt::format("`{}' expected one argument, but was called with {}", op_name, exp_count), x.constList()[0]);
            page(p).emplace_back(op);
        }
        else if (isTernaryInst(op))
        {
            if (exp_count != 3)
                buildAndThrowError(fmt::format("`{}' expected three arguments, but was called with {}", op_name, exp_count), x.constList()[0]);
            page(p).emplace_back(op);
        }
        else if (exp_count <= 1)
            buildAndThrowError(fmt::format("`{}' expected two arguments, but was called with {}", op_name, exp_count), x.constList()[0]);

        // need to check we didn't push the (op A B C D...) things for operators not supporting it
        if (exp_count > 2 && !isRepeatableOperation(op) && !isTernaryInst(op))
            buildAndThrowError(fmt::format("`{}' requires 2 arguments, but got {}.", op_name, exp_count), x);

        page(p).back().setSourceLocation(x.filename(), x.position().start.line);
    }

    bool ASTLowerer::handleFunctionCall(Node& x, const Page p, const bool is_terminal)
    {
        constexpr std::size_t start_index = 1;
        Node& node = x.list()[0];

        // number of arguments
        std::size_t args_count = 0;
        for (auto it = x.constList().begin() + start_index, it_end = x.constList().end(); it != it_end; ++it)
        {
            if (it->nodeType() != NodeType::Capture && !isBreakpoint(*it))
                args_count++;
        }

        if (is_terminal && node.nodeType() == NodeType::Symbol && isFunctionCallingItself(node.string()))
        {
            pushFunctionCallArguments(x, p, /* is_tail_call= */ true);

            if (const std::size_t expected_arg_count = m_opened_vars.top().argument_count; args_count != expected_arg_count)
            {
                std::vector<std::string> arg_names;
                if (expected_arg_count > 0)
                {
                    arg_names.reserve(expected_arg_count + 1);
                    arg_names.emplace_back("");
                    for (std::size_t i = 0; i < expected_arg_count; ++i)
                        arg_names.emplace_back(1, static_cast<char>('a' + i));
                }

                buildAndThrowError(
                    fmt::format(
                        "When performing tail-call `{}', received {} argument{}, but expected {}: `({}{})'",
                        x.repr(),
                        args_count,
                        args_count > 1 ? "s" : "",
                        expected_arg_count,
                        node.string(),
                        fmt::join(arg_names, " ")),
                    x);
            }

            // jump to the top of the function
            page(p).emplace_back(TAIL_CALL_SELF);
            page(p).back().setSourceLocation(node.filename(), node.position().start.line);
            return true;  // skip the potential Instruction::POP at the end
        }

        if (!nodeProducesOutput(node))
            buildAndThrowError(fmt::format("Can not call `{}', as it doesn't return a value", node.repr()), node);

        const IR::Entity label_return = IR::Entity::Label(m_current_label++);
        page(p).emplace_back(IR::Entity::Goto(label_return, PUSH_RETURN_ADDRESS));
        page(p).back().setSourceLocation(x.filename(), x.position().start.line);

        const Page proc_page = createNewCodePage(/* temp= */ true);
        CallType call_type = CallType::Classic;
        std::optional<uint16_t> call_arg = std::nullopt;

        // compile the function resolution to a separate page
        if (node.nodeType() == NodeType::Symbol && isFunctionCallingItself(node.string()))
        {
            // The function is trying to call itself, but this isn't a tail call.
            // We can skip the LOAD_FAST function_name and directly push the current
            // function page, which will be quicker than a local variable resolution.
            // We set its argument to the symbol id of the function we are calling,
            // so that the VM knows the name of the last called function.
            call_type = CallType::SelfNotRecursive;
        }
        else
        {
            // closure chains have been handled (eg: closure.field.field.function)
            compileExpression(node, proc_page, false, false);

            if (page(proc_page).empty())
                buildAndThrowError(fmt::format("Can not call {}", x.constList()[0].repr()), x);
            else if (page(proc_page).back().inst() == GET_FIELD)
                // the last GET_FIELD instruction should push the closure environment with it
                page(proc_page).back().replaceInstruction(GET_FIELD_AS_CLOSURE);
            else if (page(proc_page).size() == 1)
            {
                const Instruction inst = page(proc_page).back().inst();
                const uint16_t arg = page(proc_page).back().primaryArg();

                if (inst == LOAD_FAST)
                {
                    call_type = CallType::Symbol;
                    call_arg = arg;
                    // we don't want to push any instruction, as we'll use an optimised instruction instead of CALL
                    page(proc_page).clear();
                }
                else if (inst == LOAD_FAST_BY_INDEX)
                {
                    call_type = CallType::SymbolByIndex;
                    page(proc_page).clear();
                }
                else if (inst == BUILTIN && Builtins::builtins[arg].second.isFunction())
                {
                    call_type = CallType::Builtin;
                    call_arg = arg;
                    page(proc_page).clear();
                }
            }
        }

        // push proc from temp page
        for (const auto& inst : page(proc_page))
            page(p).push_back(inst);
        m_temp_pages.pop_back();

        pushFunctionCallArguments(x, p, /* is_tail_call= */ false);

        // call the procedure
        switch (call_type)
        {
            case CallType::Classic:
                page(p).emplace_back(CALL, args_count);
                break;

            case CallType::SelfNotRecursive:
                page(p).emplace_back(CALL_CURRENT_PAGE, addSymbol(node), args_count);
                break;

            case CallType::Symbol:
                assert(call_arg.has_value() && "Expected a value for call_arg with CallType::Symbol");
                page(p).emplace_back(CALL_SYMBOL, call_arg.value(), args_count);
                break;

            case CallType::SymbolByIndex:
            {
                const Page temp_page = createNewCodePage(/* temp= */ true);
                compileExpression(node, temp_page, false, false);
                assert(page(temp_page).size() == 1 && page(temp_page).back().inst() == LOAD_FAST_BY_INDEX);
                page(p).emplace_back(CALL_SYMBOL_BY_INDEX, page(temp_page).back().primaryArg(), args_count);
                m_temp_pages.pop_back();
                break;
            }

            case CallType::Builtin:
                assert(call_arg.has_value() && "Expected a value for call_arg with CallType::Builtin");
                page(p).emplace_back(CALL_BUILTIN, call_arg.value(), args_count);
                break;
        }
        page(p).back().setSourceLocation(node.filename(), node.position().start.line);

        // patch the PUSH_RETURN_ADDRESS instruction with the return location (IP=CALL instruction IP)
        page(p).emplace_back(label_return);
        return false;  // we didn't compile a tail call
    }

    uint16_t ASTLowerer::addSymbol(const Node& sym)
    {
        // otherwise, add the symbol, and return its id in the table
        auto it = std::ranges::find(m_symbols, sym.string());
        if (it == m_symbols.end())
        {
            m_symbols.push_back(sym.string());
            it = m_symbols.begin() + static_cast<std::vector<std::string>::difference_type>(m_symbols.size() - 1);
        }

        const auto distance = std::distance(m_symbols.begin(), it);
        if (std::cmp_less(distance, MaxValue16Bits))
            return static_cast<uint16_t>(distance);
        buildAndThrowError(fmt::format("Too many symbols (exceeds {}), aborting compilation.", MaxValue16Bits), sym);
    }

    uint16_t ASTLowerer::addValue(const Node& x)
    {
        const ValTableElem v(x);
        auto it = std::ranges::find(m_values, v);
        if (it == m_values.end())
        {
            m_values.push_back(v);
            it = m_values.begin() + static_cast<std::vector<ValTableElem>::difference_type>(m_values.size() - 1);
        }

        const auto distance = std::distance(m_values.begin(), it);
        if (std::cmp_less(distance, MaxValue16Bits))
            return static_cast<uint16_t>(distance);
        buildAndThrowError(fmt::format("Too many values (exceeds {}), aborting compilation.", MaxValue16Bits), x);
    }

    uint16_t ASTLowerer::addValue(const std::size_t page_id, const Node& current)
    {
        const ValTableElem v(page_id);
        auto it = std::ranges::find(m_values, v);
        if (it == m_values.end())
        {
            m_values.push_back(v);
            it = m_values.begin() + static_cast<std::vector<ValTableElem>::difference_type>(m_values.size() - 1);
        }

        const auto distance = std::distance(m_values.begin(), it);
        if (std::cmp_less(distance, MaxValue16Bits))
            return static_cast<uint16_t>(distance);
        buildAndThrowError(fmt::format("Too many values (exceeds {}), aborting compilation.", MaxValue16Bits), current);
    }
}
