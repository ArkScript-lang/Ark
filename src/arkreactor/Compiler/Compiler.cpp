#include <Ark/Compiler/Compiler.hpp>

#include <chrono>
#include <limits>
#include <utility>
#include <filesystem>
#include <algorithm>
#include <fmt/core.h>
#include <fmt/color.h>

#include <Ark/Constants.hpp>
#include <Ark/Literals.hpp>
#include <Ark/Utils.hpp>
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Compiler/Macros/Processor.hpp>

namespace Ark::internal
{
    using namespace literals;

    Compiler::Compiler(const unsigned debug) :
        m_debug(debug)
    {}

    void Compiler::process(const Node& ast)
    {
        m_code_pages.emplace_back();  // create empty page

        // gather symbols, values, and start to create code segments
        compileExpression(
            ast,
            /* current_page */ Page { .index = 0, .is_temp = false },
            /* is_result_unused */ false,
            /* is_terminal */ false);
    }

    const std::vector<IR::Block>& Compiler::intermediateRepresentation() const noexcept
    {
        return m_code_pages;
    }

    const std::vector<std::string>& Compiler::symbols() const noexcept
    {
        return m_symbols;
    }

    const std::vector<ValTableElem>& Compiler::values() const noexcept
    {
        return m_values;
    }

    std::optional<Instruction> Compiler::getOperator(const std::string& name) noexcept
    {
        const auto it = std::ranges::find(Language::operators, name);
        if (it != Language::operators.end())
            return static_cast<Instruction>(std::distance(Language::operators.begin(), it) + FIRST_OPERATOR);
        return std::nullopt;
    }

    std::optional<uint16_t> Compiler::getBuiltin(const std::string& name) noexcept
    {
        const auto it = std::ranges::find_if(Builtins::builtins,
                                             [&name](const std::pair<std::string, Value>& element) -> bool {
                                                 return name == element.first;
                                             });
        if (it != Builtins::builtins.end())
            return static_cast<uint16_t>(std::distance(Builtins::builtins.begin(), it));
        return std::nullopt;
    }

    std::optional<Instruction> Compiler::getListInstruction(const std::string& name) noexcept
    {
        const auto it = std::ranges::find(Language::listInstructions, name);
        if (it != Language::listInstructions.end())
            return static_cast<Instruction>(std::distance(Language::listInstructions.begin(), it) + LIST);
        return std::nullopt;
    }

    bool Compiler::nodeProducesOutput(const Node& node)
    {
        if (node.nodeType() == NodeType::List && !node.constList().empty() && node.constList()[0].nodeType() == NodeType::Keyword)
            return (node.constList()[0].keyword() == Keyword::Begin && node.constList().size() > 1) ||
                node.constList()[0].keyword() == Keyword::Fun ||
                node.constList()[0].keyword() == Keyword::If;
        return true;  // any other node, function call, symbol, number...
    }

    bool Compiler::isUnaryInst(const Instruction inst) noexcept
    {
        switch (inst)
        {
            case NOT: [[fallthrough]];
            case LEN: [[fallthrough]];
            case EMPTY: [[fallthrough]];
            case TAIL: [[fallthrough]];
            case HEAD: [[fallthrough]];
            case ISNIL: [[fallthrough]];
            case TO_NUM: [[fallthrough]];
            case TO_STR: [[fallthrough]];
            case TYPE:
                return true;

            default:
                return false;
        }
    }

    void Compiler::compilerWarning(const std::string& message, const Node& node)
    {
        fmt::println("{} {}", fmt::styled("Warning", fmt::fg(fmt::color::dark_orange)), Diagnostics::makeContextWithNode(message, node));
    }

    void Compiler::throwCompilerError(const std::string& message, const Node& node)
    {
        throw CodeError(message, node.filename(), node.line(), node.col(), node.repr());
    }

    void Compiler::compileExpression(const Node& x, const Page p, const bool is_result_unused, const bool is_terminal, const std::string& var_name)
    {
        // register symbols
        if (x.nodeType() == NodeType::Symbol)
            compileSymbol(x, p, is_result_unused);
        else if (x.nodeType() == NodeType::Field)
        {
            // the parser guarantees us that there is at least 2 elements (eg: a.b)
            compileSymbol(x.constList()[0], p, is_result_unused);
            for (auto it = x.constList().begin() + 1, end = x.constList().end(); it != end; ++it)
            {
                uint16_t i = addSymbol(*it);
                page(p).emplace_back(GET_FIELD, i);
            }
        }
        // register values
        else if (x.nodeType() == NodeType::String || x.nodeType() == NodeType::Number)
        {
            uint16_t i = addValue(x);

            if (!is_result_unused)
                page(p).emplace_back(LOAD_CONST, i);
        }
        // empty code block should be nil
        else if (x.constList().empty())
        {
            if (!is_result_unused)
            {
                static const std::optional<uint16_t> nil = getBuiltin("nil");
                page(p).emplace_back(BUILTIN, nil.value());
            }
        }
        // list instructions
        else if (const auto c0 = x.constList()[0]; c0.nodeType() == NodeType::Symbol && getListInstruction(c0.string()).has_value())
            compileListInstruction(c0, x, p, is_result_unused);
        // registering structures
        else if (x.constList()[0].nodeType() == NodeType::Keyword)
        {
            switch (const Keyword keyword = x.constList()[0].keyword())
            {
                case Keyword::If:
                    compileIf(x, p, is_result_unused, is_terminal, var_name);
                    break;

                case Keyword::Set:
                    [[fallthrough]];
                case Keyword::Let:
                    [[fallthrough]];
                case Keyword::Mut:
                    compileLetMutSet(keyword, x, p);
                    break;

                case Keyword::Fun:
                    compileFunction(x, p, is_result_unused, var_name);
                    break;

                case Keyword::Begin:
                {
                    for (std::size_t i = 1, size = x.constList().size(); i < size; ++i)
                        compileExpression(
                            x.constList()[i],
                            p,
                            // All the nodes in a begin (except for the last one) are producing a result that we want to drop.
                            (i != size - 1) || is_result_unused,
                            // If the begin is a terminal node, only its last node is terminal.
                            is_terminal && (i == size - 1),
                            var_name);
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
                    break;
            }
        }
        else if (x.nodeType() == NodeType::List)
        {
            // if we are here, we should have a function name
            // push arguments first, then function name, then call it
            handleCalls(x, p, is_result_unused, is_terminal, var_name);
        }
        else
            throwCompilerError("boop", x);  // FIXME
    }

    void Compiler::compileSymbol(const Node& x, const Page p, const bool is_result_unused)
    {
        const std::string& name = x.string();

        if (const auto it_builtin = getBuiltin(name))
            page(p).emplace_back(Instruction::BUILTIN, it_builtin.value());
        else if (getOperator(name).has_value())
            throwCompilerError(fmt::format("Found a free standing operator: `{}`", name), x);
        else
            page(p).emplace_back(LOAD_SYMBOL, addSymbol(x));  // using the variable

        if (is_result_unused)
        {
            compilerWarning("Statement has no effect", x);
            page(p).emplace_back(POP);
        }
    }

    void Compiler::compileListInstruction(const Node& c0, const Node& x, const Page p, const bool is_result_unused)
    {
        std::string name = c0.string();
        Instruction inst = getListInstruction(name).value();

        // length of at least 1 since we got a symbol name
        const auto argc = x.constList().size() - 1u;
        // error, can not use append/concat/pop (and their in place versions) with a <2 length argument list
        if (argc < 2 && inst != LIST)
            throwCompilerError(fmt::format("Can not use {} with less than 2 arguments", name), c0);
        if (std::cmp_greater(argc, std::numeric_limits<uint16_t>::max()))
            throwCompilerError(fmt::format("Too many arguments ({}), exceeds 65'535", argc), x);

        // compile arguments in reverse order
        for (std::size_t i = x.constList().size() - 1u; i > 0; --i)
        {
            const auto node = x.constList()[i];
            if (nodeProducesOutput(node))
                compileExpression(node, p, false, false);
            else
                throwCompilerError(fmt::format("Invalid node inside call to {}", name), node);
        }

        // put inst and number of arguments
        std::size_t inst_argc;
        switch (inst)
        {
            case LIST:
                inst_argc = argc;
                break;

            case APPEND:
            case APPEND_IN_PLACE:
            case CONCAT:
            case CONCAT_IN_PLACE:
                inst_argc = argc - 1;
                break;

            default:
                inst_argc = 0;
                break;
        }
        page(p).emplace_back(inst, static_cast<uint16_t>(inst_argc));

        if (is_result_unused && name.back() != '!')  // in-place functions never push a value
        {
            compilerWarning("Ignoring return value of function", x);
            page(p).emplace_back(POP);
        }
    }

    void Compiler::compileIf(const Node& x, const Page p, const bool is_result_unused, const bool is_terminal, const std::string& var_name)
    {
        // compile condition
        compileExpression(x.constList()[1], p, false, false);

        // jump only if needed to the if
        const auto label_then = IR::Entity::Label();
        page(p).emplace_back(IR::Entity::GotoIf(label_then, true));

        // else code
        if (x.constList().size() == 4)  // we have an else clause
            compileExpression(x.constList()[3], p, is_result_unused, is_terminal, var_name);

        // when else is finished, jump to end
        const auto label_end = IR::Entity::Label();
        page(p).emplace_back(IR::Entity::Goto(label_end));

        // absolute address to jump to if condition is true
        page(p).emplace_back(label_then);
        // if code
        compileExpression(x.constList()[2], p, is_result_unused, is_terminal, var_name);
        // set jump to end pos
        page(p).emplace_back(label_end);
    }

    void Compiler::compileFunction(const Node& x, const Page p, const bool is_result_unused, const std::string& var_name)
    {
        if (const auto args = x.constList()[1]; args.nodeType() != NodeType::List)
            throwCompilerError(fmt::format("Expected a well formed argument(s) list, got a {}", typeToString(args)), args);
        if (x.constList().size() != 3)
            throwCompilerError("Invalid node ; if it was computed by a macro, check that a node is returned", x);

        // capture, if needed
        bool is_closure = false;
        for (const auto& node : x.constList()[1].constList())
        {
            if (node.nodeType() == NodeType::Capture)
            {
                page(p).emplace_back(CAPTURE, addSymbol(node));
                is_closure = true;
            }
        }

        // create new page for function body
        m_code_pages.emplace_back();
        const auto function_body_page = Page { .index = m_code_pages.size() - 1, .is_temp = false };
        // save page_id into the constants table as PageAddr and load the const
        page(p).emplace_back(is_closure ? MAKE_CLOSURE : LOAD_CONST, addValue(function_body_page.index, x));

        // pushing arguments from the stack into variables in the new scope
        for (const auto& node : x.constList()[1].constList())
        {
            if (node.nodeType() == NodeType::Symbol)
                page(function_body_page).emplace_back(STORE, addSymbol(node));
        }

        // push body of the function
        compileExpression(x.constList()[2], function_body_page, false, true, var_name);

        // return last value on the stack
        page(function_body_page).emplace_back(RET);

        // if the computed function is unused, pop it
        if (is_result_unused)
        {
            compilerWarning("Unused declared function", x);
            page(p).emplace_back(POP);
        }
    }

    void Compiler::compileLetMutSet(const Keyword n, const Node& x, const Page p)
    {
        if (const auto sym = x.constList()[1]; sym.nodeType() != NodeType::Symbol)
            throwCompilerError(fmt::format("Expected a symbol, got a {}", typeToString(sym)), sym);
        if (x.constList().size() != 3)
            throwCompilerError("Invalid node ; if it was computed by a macro, check that a node is returned", x);

        const std::string name = x.constList()[1].string();
        uint16_t i = addSymbol(x.constList()[1]);

        // put value before symbol id
        // starting at index = 2 because x is a (let|mut|set variable ...) node
        for (std::size_t idx = 2, end = x.constList().size(); idx < end; ++idx)
            compileExpression(x.constList()[idx], p, false, false, name);

        if (n == Keyword::Let || n == Keyword::Mut)
            page(p).emplace_back(STORE, i);
        else
            page(p).emplace_back(SET_VAL, i);
    }

    void Compiler::compileWhile(const Node& x, const Page p)
    {
        if (x.constList().size() != 3)
            throwCompilerError("Invalid node ; if it was computed by a macro, check that a node is returned", x);

        // save current position to jump there at the end of the loop
        const auto label_loop = IR::Entity::Label();
        page(p).emplace_back(label_loop);
        // push condition
        compileExpression(x.constList()[1], p, false, false);
        // absolute jump to end of block if condition is false
        const auto label_end = IR::Entity::Label();
        page(p).emplace_back(IR::Entity::GotoIf(label_end, false));
        // push code to page
        compileExpression(x.constList()[2], p, true, false);

        // loop, jump to the condition
        page(p).emplace_back(IR::Entity::Goto(label_loop));

        // absolute address to jump to if condition is false
        page(p).emplace_back(label_end);
    }

    void Compiler::compilePluginImport(const Node& x, const Page p)
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
    }

    void Compiler::handleCalls(const Node& x, const Page p, bool is_result_unused, const bool is_terminal, const std::string& var_name)
    {
        constexpr std::size_t start_index = 1;

        const auto node = x.constList()[0];
        const std::optional<Instruction> maybe_operator = node.nodeType() == NodeType::Symbol ? getOperator(node.string()) : std::nullopt;

        enum class ShortcircuitOp
        {
            And,
            Or
        };
        const std::optional<ShortcircuitOp> maybe_shortcircuit =
            node.nodeType() == NodeType::Symbol
            ? (node.string() == Language::And
                   ? std::make_optional(ShortcircuitOp::And)
                   : (node.string() == Language::Or
                          ? std::make_optional(ShortcircuitOp::Or)
                          : std::nullopt))
            : std::nullopt;

        if (maybe_shortcircuit.has_value())
        {
            // short circuit implementation

            compileExpression(x.constList()[1], p, false, false);
            page(p).emplace_back(DUP);

            const auto label_shortcircuit = IR::Entity::Label();
            for (std::size_t i = 2, end = x.constList().size(); i < end; ++i)
            {
                switch (maybe_shortcircuit.value())
                {
                    case ShortcircuitOp::And:
                        page(p).emplace_back(IR::Entity::GotoIf(label_shortcircuit, false));
                        break;
                    case ShortcircuitOp::Or:
                        page(p).emplace_back(IR::Entity::GotoIf(label_shortcircuit, true));
                        break;
                }
                page(p).emplace_back(POP);

                compileExpression(x.constList()[i], p, false, false);
                if (i + 1 != end)
                    page(p).emplace_back(DUP);
            }

            page(p).emplace_back(label_shortcircuit);
        }
        else if (!maybe_operator.has_value())
        {
            if (is_terminal && x.constList()[0].nodeType() == NodeType::Symbol && var_name == x.constList()[0].string())
            {
                // push the arguments in reverse order
                for (std::size_t i = x.constList().size() - 1; i >= start_index; --i)
                {
                    if (nodeProducesOutput(x.constList()[i]))
                        compileExpression(x.constList()[i], p, false, false);
                    else
                        throwCompilerError(fmt::format("Invalid node inside tail call to `{}'", node.repr()), x);
                }

                // jump to the top of the function
                page(p).emplace_back(JUMP, 0_u16);
                return;  // skip the potential Instruction::POP at the end
            }
            else
            {
                m_temp_pages.emplace_back();
                const auto proc_page = Page { .index = m_temp_pages.size() - 1u, .is_temp = true };
                // closure chains have been handled (eg: closure.field.field.function)
                compileExpression(node, proc_page, false, false);  // storing proc
                if (m_temp_pages.back().empty())
                    throwCompilerError(fmt::format("Can not call {}", x.constList()[0].repr()), x);

                // push arguments on current page
                for (auto exp = x.constList().begin() + start_index, exp_end = x.constList().end(); exp != exp_end; ++exp)
                {
                    if (nodeProducesOutput(*exp))
                        compileExpression(*exp, p, false, false);
                    else
                        throwCompilerError(fmt::format("Invalid node inside call to `{}'", node.repr()), x);
                }
                // push proc from temp page
                for (const auto& inst : m_temp_pages.back())
                    page(p).push_back(inst);
                m_temp_pages.pop_back();

                // number of arguments
                std::size_t args_count = 0;
                for (auto it = x.constList().begin() + 1, it_end = x.constList().end(); it != it_end; ++it)
                {
                    if (it->nodeType() != NodeType::Capture)
                        args_count++;
                }
                // call the procedure
                page(p).emplace_back(CALL, args_count);
            }
        }
        else  // operator
        {
            // retrieve operator
            auto op = maybe_operator.value();

            if (op == ASSERT)
                is_result_unused = false;

            // push arguments on current page
            std::size_t exp_count = 0;
            for (std::size_t index = start_index, size = x.constList().size(); index < size; ++index)
            {
                if (nodeProducesOutput(x.constList()[index]))
                    compileExpression(x.constList()[index], p, false, false);
                else
                    throwCompilerError(fmt::format("Invalid node inside call to operator `{}'", node.repr()), x);

                if ((index + 1 < size && x.constList()[index + 1].nodeType() != NodeType::Capture) || index + 1 == size)
                    exp_count++;

                // in order to be able to handle things like (op A B C D...)
                // which should be transformed into A B op C op D op...
                if (exp_count >= 2)
                    page(p).emplace_back(op);
            }

            if (isUnaryInst(op))
            {
                if (exp_count != 1)
                    throwCompilerError(fmt::format("Operator needs one argument, but was called with {}", exp_count), x.constList()[0]);
                page(p).emplace_back(op);
            }
            else if (exp_count <= 1)
            {
                throwCompilerError(fmt::format("Operator needs two arguments, but was called with {}", exp_count), x.constList()[0]);
            }

            // need to check we didn't push the (op A B C D...) things for operators not supporting it
            if (exp_count > 2)
            {
                switch (op)
                {
                    // authorized instructions
                    case ADD: [[fallthrough]];
                    case SUB: [[fallthrough]];
                    case MUL: [[fallthrough]];
                    case DIV: [[fallthrough]];
                    case MOD:
                        break;

                    default:
                        throwCompilerError(
                            fmt::format(
                                "can not create a chained expression (of length {}) for operator `{}'. You most likely forgot a `)'.",
                                exp_count,
                                Language::operators[static_cast<std::size_t>(op - FIRST_OPERATOR)]),
                            x);
                }
            }
        }

        if (is_result_unused)
            page(p).emplace_back(POP);
    }

    uint16_t Compiler::addSymbol(const Node& sym)
    {
        // otherwise, add the symbol, and return its id in the table
        auto it = std::ranges::find(m_symbols, sym.string());
        if (it == m_symbols.end())
        {
            m_symbols.push_back(sym.string());
            it = m_symbols.begin() + static_cast<std::vector<std::string>::difference_type>(m_symbols.size() - 1);
        }

        const auto distance = std::distance(m_symbols.begin(), it);
        if (distance < std::numeric_limits<uint16_t>::max())
            return static_cast<uint16_t>(distance);
        throwCompilerError("Too many symbols (exceeds 65'536), aborting compilation.", sym);
    }

    uint16_t Compiler::addValue(const Node& x)
    {
        const ValTableElem v(x);
        auto it = std::ranges::find(m_values, v);
        if (it == m_values.end())
        {
            m_values.push_back(v);
            it = m_values.begin() + static_cast<std::vector<ValTableElem>::difference_type>(m_values.size() - 1);
        }

        const auto distance = std::distance(m_values.begin(), it);
        if (distance < std::numeric_limits<uint16_t>::max())
            return static_cast<uint16_t>(distance);
        throwCompilerError("Too many values (exceeds 65'536), aborting compilation.", x);
    }

    uint16_t Compiler::addValue(const std::size_t page_id, const Node& current)
    {
        const ValTableElem v(page_id);
        auto it = std::ranges::find(m_values, v);
        if (it == m_values.end())
        {
            m_values.push_back(v);
            it = m_values.begin() + static_cast<std::vector<ValTableElem>::difference_type>(m_values.size() - 1);
        }

        const auto distance = std::distance(m_values.begin(), it);
        if (distance < std::numeric_limits<uint16_t>::max())
            return static_cast<uint16_t>(distance);
        throwCompilerError("Too many values (exceeds 65'536), aborting compilation.", current);
    }
}
