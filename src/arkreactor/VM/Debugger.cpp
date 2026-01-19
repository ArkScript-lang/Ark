#include <Ark/VM/Debugger.hpp>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

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
        saveState(context);
    }

    Debugger::Debugger(const std::vector<std::filesystem::path>& libenv, const std::string& path_to_prompt_file, std::ostream& os, const std::vector<std::string>& symbols, const std::vector<Value>& constants) :
        m_libenv(libenv),
        m_symbols(symbols),
        m_constants(constants),
        m_os(os),
        m_colorize(false),
        m_prompt_stream(std::make_unique<std::ifstream>(path_to_prompt_file))
    {}

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

    void Debugger::run(VM& vm, ExecutionContext& context)
    {
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
            std::optional<std::string> maybe_input = prompt(ip_at_breakpoint, pp_at_breakpoint);

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
                        if (maybe_value != nullptr && maybe_value->valueType() != ValueType::Undefined && maybe_value->valueType() != ValueType::InstPtr)
                            fmt::println(
                                m_os,
                                "{}",
                                fmt::styled(
                                    maybe_value->toString(vm),
                                    m_colorize ? fmt::fg(fmt::color::chocolate) : fmt::text_style()));
                    }
                }
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
                        .end = std::nullopt },
                    m_os,
                    /* maybe_context= */ std::nullopt,
                    /* colorize= */ m_colorize);
                fmt::println(m_os, "");
            }
        }
    }

    std::optional<std::string> Debugger::prompt(const std::size_t ip, const std::size_t pp)
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
                fmt::format("ip:{}", fmt::styled(ip, m_colorize ? fmt::fg(fmt::color::cyan) : fmt::text_style())),
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

            if (line == "c" || line == "continue" || line.empty())
            {
                fmt::println(m_os, "dbg: continue");
                return std::nullopt;
            }
            else if (line == "q" || line == "quit")
            {
                fmt::println(m_os, "dbg: stop");
                m_quit_vm = true;
                return std::nullopt;
            }
            else if (line == "help")
            {
                fmt::println(m_os, "Available commands:");
                fmt::println(m_os, "  help -- display this message");
                fmt::println(m_os, "  c, continue -- resume execution");
                fmt::println(m_os, "  q, quit -- quit the debugger, stopping the script execution");
            }
            else
            {
                code += line;

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
