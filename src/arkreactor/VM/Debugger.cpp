#include <Ark/VM/Debugger.hpp>

#include <fmt/core.h>
#include <fmt/color.h>

#include <Ark/State.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/Utils/Files.hpp>
#include <Ark/Compiler/Welder.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Error/Diagnostics.hpp>

namespace Ark::internal
{
    Debugger::Debugger(const ExecutionContext& context, const std::vector<std::filesystem::path>& libenv, const std::vector<std::string>& symbols, const std::vector<Value>& constants) :
        m_libenv(libenv), m_symbols(symbols), m_constants(constants), m_running(false), m_quit_vm(false)
    {
        saveState(context);
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

    void Debugger::run(VM& vm, ExecutionContext& context)
    {
        m_running = true;
        const bool is_vm_running = vm.m_running;

        // show the line where the breakpoint hit
        const auto maybe_source_loc = vm.findSourceLocation(context.ip, context.pp);
        if (maybe_source_loc)
        {
            const auto filename = vm.m_state.m_filenames[maybe_source_loc->filename_id];

            if (Utils::fileExists(filename))
            {
                fmt::println("");
                Diagnostics::makeContext(
                    Diagnostics::ErrorLocation {
                        .filename = filename,
                        .start = FilePos { .line = maybe_source_loc->line, .column = 0 },
                        .end = std::nullopt },
                    std::cout,
                    /* maybe_context= */ std::nullopt,
                    /* colorize= */ true);
                fmt::println("");
            }
        }

        while (true)
        {
            std::optional<std::string> maybe_input = prompt();

            if (maybe_input)
            {
                const std::string& line = maybe_input.value();

                if (const auto pages = compile(m_code + line, vm.m_state.m_pages.size()); pages.has_value())
                {
                    context.ip = 0;
                    context.pp = vm.m_state.m_pages.size();
                    // create dedicated scope, so that we won't be overwriting existing variables
                    context.locals.emplace_back(context.scopes_storage.data(), context.locals.back().storageEnd());

                    vm.m_state.extendBytecode(pages.value(), m_symbols, m_constants);

                    if (vm.safeRun(context) == 0)
                    {
                        // executing code worked
                        m_code += line;

                        const Value* maybe_value = vm.peekAndResolveAsPtr(context);
                        if (maybe_value != nullptr && maybe_value->valueType() != ValueType::Undefined && maybe_value->valueType() != ValueType::InstPtr)
                            fmt::println("{}", fmt::styled(maybe_value->toString(vm), fmt::fg(fmt::color::chocolate)));
                    }

                    context.locals.pop_back();
                }
            }
            else
                break;
        }

        m_running = false;
        // we do not want to retain code from the past executions
        m_code.clear();
        // we hit a HALT instruction that set 'running' to false, ignore that if we were still running!
        vm.m_running = is_vm_running;
    }

    std::optional<std::string> Debugger::prompt()
    {
        std::string code;
        long open_parens = 0;
        long open_braces = 0;

        while (true)
        {
            const bool unfinished_block = open_parens != 0 || open_braces != 0;
            fmt::print("dbg:{:0>3}{} ", m_line_count, unfinished_block ? ":" : ">");
            std::string line;
            std::getline(std::cin, line);

            Utils::trimWhitespace(line);

            if (line == "c" || line == "continue" || line.empty())
            {
                fmt::println("dbg: continue");
                return std::nullopt;
            }
            else if (line == "q" || line == "quit")
            {
                fmt::println("dbg: stop");
                m_quit_vm = true;
                return std::nullopt;
            }
            else if (line == "help")
            {
                fmt::println("Available commands:");
                fmt::println("  help -- display this message");
                fmt::println("  c, continue -- resume execution");
                fmt::println("  q, quit -- quit the debugger, stopping the script execution");
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

    std::optional<std::vector<bytecode_t>> Debugger::compile(const std::string& code, const std::size_t start_page_at_offset)
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

        m_symbols = syms.symbols;
        m_constants = vals.values;

        return pages;
    }
}
