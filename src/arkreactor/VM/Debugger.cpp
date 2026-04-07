#include <Ark/VM/Debugger.hpp>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ostream.h>
#include <chrono>
#include <thread>
#include <charconv>

#include <Ark/State.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/Utils/Files.hpp>
#include <Ark/Compiler/Welder.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Error/Diagnostics.hpp>

namespace Ark::internal
{
    Debugger::Debugger(const ExecutionContext& context, const std::vector<std::filesystem::path>& libenv, const std::vector<std::string>& symbols, const std::vector<Value>& constants) :
        m_libenv(libenv),
        m_symbols(symbols),
        m_constants(constants),
        m_os(std::cout),
        m_colorize(true)
    {
        initCommands();
        saveState(context);
    }

    Debugger::Debugger(const std::vector<std::filesystem::path>& libenv, const std::string& path_to_prompt_file, std::ostream& os, const std::vector<std::string>& symbols, const std::vector<Value>& constants) :
        m_libenv(libenv),
        m_symbols(symbols),
        m_constants(constants),
        m_os(os),
        m_colorize(false),
        m_prompt_stream(std::make_unique<std::ifstream>(path_to_prompt_file))
    {
        initCommands();
    }

    void Debugger::saveState(const ExecutionContext& context)
    {
        m_states.emplace_back(
            std::make_unique<SavedState>(
                context.ip,
                context.pp,
                context.sp,
                context.fc,
                context.locals,
                context.stacked_closure_scopes));
    }

    void Debugger::resetContextToSavedState(ExecutionContext& context)
    {
        const auto& [ip, pp, sp, fc, locals, closure_scopes] = *m_states.back();
        context.locals = locals;
        context.stacked_closure_scopes = closure_scopes;
        context.ip = ip;
        context.pp = pp;
        context.sp = sp;
        context.fc = fc;

        m_states.pop_back();
    }

    void Debugger::run(VM& vm, ExecutionContext& context, const bool from_breakpoint)
    {
        using namespace std::chrono_literals;

        if (from_breakpoint)
            showContext(vm, context);

        m_running = true;
        const bool is_vm_running = vm.m_running;
        const std::size_t ip_at_breakpoint = context.ip,
                          pp_at_breakpoint = context.pp;
        // create dedicated scope, so that we won't be overwriting existing variables
        context.locals.emplace_back(context.scopes_storage.data(), context.locals.back().storageEnd());
        std::size_t last_ip = 0;

        while (true)
        {
            std::optional<std::string> maybe_input = prompt(ip_at_breakpoint, pp_at_breakpoint, vm, context);

            if (maybe_input)
            {
                const std::string& line = maybe_input.value();

                if (const auto compiled = compile(m_code + line, vm.m_state.m_pages.size()); compiled.has_value())
                {
                    context.ip = last_ip;
                    context.pp = vm.m_state.m_pages.size();

                    vm.m_state.extendBytecode(compiled->pages, compiled->symbols, compiled->constants);

                    if (vm.safeRun(context) == 0)
                    {
                        // executing code worked
                        m_code += line + "\n";
                        // place ip to end of bytecode instruction (HALT)
                        last_ip = context.ip - 4;

                        const Value* maybe_value = vm.peekAndResolveAsPtr(context);
                        if (maybe_value != nullptr &&
                            maybe_value->valueType() != ValueType::Undefined &&
                            maybe_value->valueType() != ValueType::InstPtr &&
                            maybe_value->valueType() != ValueType::Garbage)
                            fmt::println(
                                m_os,
                                "{}",
                                fmt::styled(
                                    maybe_value->toString(vm),
                                    m_colorize ? fmt::fg(fmt::color::chocolate) : fmt::text_style()));
                    }
                }
                else
                    std::this_thread::sleep_for(50ms);  // hack to wait for the diagnostics to be output to stderr, since we write to stdout and it's faster than stderr
            }
            else
                break;
        }

        context.locals.pop_back();

        // we do not want to retain code from the past executions
        m_code.clear();
        m_line_count = 0;

        // we hit a HALT instruction that set 'running' to false, ignore that if we were still running!
        vm.m_running = is_vm_running;
        m_running = false;
    }

    void Debugger::registerInstruction(const uint32_t word) noexcept
    {
        m_previous_insts.push_back(word);
    }

    void Debugger::initCommands()
    {
        m_commands = {
            Command(
                "help",
                "display this message",
                [this](const std::string&, const CommandArgs&) {
                    fmt::println(m_os, "Available commands:");
                    for (const Command& cmd : m_commands)
                    {
                        if (cmd.is_exact)
                            fmt::println(m_os, "  {} -- {}", fmt::join(cmd.names, ", "), cmd.description);
                        else
                            // todo: make arguments description configurable
                            fmt::println(m_os, "  {} <n=5> -- {}", fmt::join(cmd.names, ", "), cmd.description);
                    }
                    return false;
                }),
            Command(
                { "c", "continue" },
                "resume execution",
                [this](const std::string&, const CommandArgs&) {
                    fmt::println(m_os, "dbg: continue");
                    return true;
                }),
            Command(
                { "q", "quit" },
                "quit the debugger, stopping the script execution",
                [this](const std::string&, const CommandArgs&) {
                    fmt::println(m_os, "dbg: stop");
                    m_quit_vm = true;
                    return true;
                }),
            Command(
                StartsWith("stack"),
                "show the last n values on the stack",
                [this](const std::string& line, const CommandArgs& args) {
                    if (const auto arg = getArgAndParseOrError("stack", line, /* default_value= */ 5))
                        showStack(*args.vm_ptr, *args.ctx_ptr, arg.value());
                    return false;
                }),
            Command(
                StartsWith("locals"),
                "show the last n values on the locals' stack",
                [this](const std::string& line, const CommandArgs& args) {
                    if (const auto arg = getArgAndParseOrError("locals", line, /* default_value= */ 5))
                        showLocals(*args.vm_ptr, *args.ctx_ptr, arg.value());
                    return false;
                }),
            Command(
                "ptr",
                "show the values of the VM pointers",
                [this](const std::string&, const CommandArgs& args) {
                    fmt::println(
                        m_os,
                        "IP: {} - PP: {} - SP: {}",
                        fmt::styled(args.ip / 4, m_colorize ? fmt::fg(fmt::color::cyan) : fmt::text_style()),
                        fmt::styled(args.pp, m_colorize ? fmt::fg(fmt::color::green) : fmt::text_style()),
                        fmt::styled(args.ctx_ptr->sp, m_colorize ? fmt::fg(fmt::color::yellow) : fmt::text_style()));
                    return false;
                }),
            Command(
                StartsWith("trace"),
                "show the last n executed instructions",
                [this](const std::string& line, const CommandArgs& args) {
                    if (const auto arg = getArgAndParseOrError("trace", line, /* default_value= */ 10))
                        showPreviousInstructions(*args.vm_ptr, arg.value());
                    return false;
                }),
        };
    }

    std::optional<Debugger::Command> Debugger::matchCommand(const std::string& line) const
    {
        for (const Command& c : m_commands)
        {
            if (c.is_exact)
            {
                if (std::ranges::find(c.names, line) != c.names.end())
                    return c;
            }
            else
            {
                if (std::ranges::find_if(c.names, [&line](const std::string& name) -> bool {
                        return line.starts_with(name);
                    }) != c.names.end())
                    return c;
            }
        }

        return std::nullopt;
    }

    void Debugger::showContext(const VM& vm, const ExecutionContext& context) const
    {
        // show the line where the breakpoint hit
        const auto maybe_source_loc = vm.findSourceLocation(context.ip, context.pp);
        if (maybe_source_loc)
        {
            const auto filename = vm.m_state.m_filenames[maybe_source_loc->filename_id];

            if (Utils::fileExists(filename))
            {
                fmt::println(m_os, "");
                Diagnostics::makeContext(
                    Diagnostics::ErrorLocation {
                        .filename = filename,
                        .start = FilePos { .line = maybe_source_loc->line, .column = 0 },
                        .end = std::nullopt,
                        .maybe_content = std::nullopt },
                    m_os,
                    /* maybe_context= */ std::nullopt,
                    /* colorize= */ m_colorize);
                fmt::println(m_os, "");
            }
        }
    }

    void Debugger::showStack(VM& vm, const ExecutionContext& context, const std::size_t count) const
    {
        std::size_t i = 1;
        do
        {
            if (context.sp < i)
                break;

            const auto color = m_colorize ? fmt::fg(i % 2 == 0 ? fmt::color::forest_green : fmt::color::cornflower_blue) : fmt::text_style();
            fmt::println(
                m_os,
                "{} -> {}",
                fmt::styled(context.sp - i, color),
                fmt::styled(context.stack[context.sp - i].toString(vm, /* show_as_code= */ true), color));
            ++i;
        } while (i < count);

        if (context.sp == 0)
            fmt::println(m_os, "Stack is empty");

        fmt::println(m_os, "");
    }

    void Debugger::showLocals(VM& vm, ExecutionContext& context, const std::size_t count) const
    {
        const std::size_t limit = context.locals[context.locals.size() - 2].size();  // -2 because we created a scope for the debugger
        if (limit > 0 && count > 0)
        {
            fmt::println(m_os, "scope size: {}", limit);
            fmt::println(m_os, "index |  id |      name      |    type   | value");
            std::size_t i = 0;

            do
            {
                if (limit <= i)
                    break;

                auto& [id, value] = context.locals[context.locals.size() - 2].atPosReverse(i);
                const auto color = m_colorize ? fmt::fg(i % 2 == 0 ? fmt::color::forest_green : fmt::color::cornflower_blue) : fmt::text_style();

                fmt::println(
                    m_os,
                    "{:>5} | {:3} | {:14} | {:>9} | {}",
                    fmt::styled(limit - i - 1, color),
                    fmt::styled(id, color),
                    fmt::styled(vm.m_state.m_symbols[id], color),
                    fmt::styled(std::to_string(value.valueType()), color),
                    fmt::styled(value.toString(vm, /* show_as_code= */ true), color));
                ++i;
            } while (i < count);
        }
        else
            fmt::println(m_os, "Current scope is empty");

        fmt::println(m_os, "");
    }

    void Debugger::showPreviousInstructions(const VM& vm, const std::size_t count) const
    {
        BytecodeReader bcr;
        bcr.feed(vm.bytecode());

        const auto syms = bcr.symbols();
        const auto vals = bcr.values(syms);

        for (std::size_t i = 0; i < count; ++i)
        {
            if (i >= m_previous_insts.size())
                break;

            const uint8_t inst = (m_previous_insts[m_previous_insts.size() - 1 - i] >> 24) & 0xff;
            const uint8_t padding = (m_previous_insts[m_previous_insts.size() - 1 - i] >> 16) & 0xff;
            const uint16_t arg = m_previous_insts[m_previous_insts.size() - 1 - i] & 0xffff;
            bcr.printInstruction(m_os, inst, padding, arg, syms, vals, m_colorize);
        }
    }

    std::optional<std::string> Debugger::getCommandArg(const std::string& command, const std::string& line)
    {
        std::string arg = line.substr(command.size());
        Utils::trimWhitespace(arg);

        if (arg.empty())
            return std::nullopt;
        return arg;
    }

    std::optional<std::size_t> Debugger::parseStringAsInt(const std::string& str)
    {
        std::size_t result = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);

        if (ec == std::errc())
            return result;
        return std::nullopt;
    }

    std::optional<std::size_t> Debugger::getArgAndParseOrError(const std::string& command, const std::string& line, const std::size_t default_value) const
    {
        const auto maybe_arg = getCommandArg(command, line);
        std::size_t count = default_value;
        if (maybe_arg)
        {
            if (const auto maybe_int = parseStringAsInt(maybe_arg.value()))
                count = maybe_int.value();
            else
            {
                fmt::println(m_os, "Couldn't parse argument as an integer");
                return std::nullopt;
            }
        }

        return count;
    }

    std::optional<std::string> Debugger::prompt(const std::size_t ip, const std::size_t pp, VM& vm, ExecutionContext& context)
    {
        std::string code;
        long open_parens = 0;
        long open_braces = 0;

        while (true)
        {
            const bool unfinished_block = open_parens != 0 || open_braces != 0;
            fmt::print(
                m_os,
                "dbg[{},{}]:{:0>3}{} ",
                fmt::format("pp:{}", fmt::styled(pp, m_colorize ? fmt::fg(fmt::color::green) : fmt::text_style())),
                fmt::format("ip:{}", fmt::styled(ip / 4, m_colorize ? fmt::fg(fmt::color::cyan) : fmt::text_style())),
                m_line_count,
                unfinished_block ? ":" : ">");

            std::string line;
            if (m_prompt_stream)
            {
                std::getline(*m_prompt_stream, line);
                fmt::println(m_os, "{}", line);  // because nothing is printed otherwise, and prompts get printed on the same line
            }
            else
                std::getline(std::cin, line);

            Utils::trimWhitespace(line);

            if (line.empty() && !unfinished_block)
            {
                fmt::println(m_os, "dbg: continue");
                return std::nullopt;
            }

            if (const auto& maybe_cmd = matchCommand(line))
            {
                if (maybe_cmd->action(line, CommandArgs { .vm_ptr = &vm, .ctx_ptr = &context, .ip = ip, .pp = pp }))
                    return std::nullopt;
            }
            else
            {
                code += line + "\n";

                open_parens += Utils::countOpenEnclosures(line, '(', ')');
                open_braces += Utils::countOpenEnclosures(line, '{', '}');

                ++m_line_count;
                if (open_braces == 0 && open_parens == 0)
                    break;
            }
        }

        return code;
    }

    std::optional<CompiledPrompt> Debugger::compile(const std::string& code, const std::size_t start_page_at_offset) const
    {
        Welder welder(0, m_libenv, DefaultFeatures);
        if (!welder.computeASTFromStringWithKnownSymbols(code, m_symbols))
            return std::nullopt;
        if (!welder.generateBytecodeUsingTables(m_symbols, m_constants, start_page_at_offset))
            return std::nullopt;

        BytecodeReader bcr;
        bcr.feed(welder.bytecode());
        const auto syms = bcr.symbols();
        const auto vals = bcr.values(syms);
        const auto files = bcr.filenames(vals);
        const auto inst_locs = bcr.instLocations(files);
        const auto [pages, _] = bcr.code(inst_locs);

        return std::optional(CompiledPrompt(pages, syms.symbols, vals.values));
    }
}
