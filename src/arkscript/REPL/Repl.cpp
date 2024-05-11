#include <fmt/core.h>

#include <CLI/REPL/Repl.hpp>
#include <CLI/REPL/Utils.hpp>

namespace Ark
{
    using namespace internal;

    Repl::Repl(const std::vector<std::filesystem::path>& lib_env) :
        m_old_ip(0), m_lib_env(lib_env), m_line_count(1), m_running(true)
    {}

    int Repl::run()
    {
        Ark::State state(m_lib_env);
        Ark::VM vm(state);
        state.setDebug(0);
        bool init = false;

        fmt::print("ArkScript REPL -- Version {} [LICENSE: Mozilla Public License 2.0]\nType \"(quit)\" to quit.\n", ARK_FULL_VERSION);
        cuiSetup();

        std::string code;
        while (m_running)
        {
            auto maybe_block = getCodeBlock();

            // save a valid ip if execution failed
            m_old_ip = vm.m_execution_contexts[0]->ip;
            if (maybe_block.has_value() && !maybe_block.value().empty())
            {
                std::string new_code = code + maybe_block.value();
                if (state.doString(new_code))
                {
                    // for only one vm init
                    if (!init)
                    {
                        vm.init();
                        init = true;
                    }

                    if (vm.safeRun(*vm.m_execution_contexts[0]) == 0)
                    {
                        // save good code
                        code = new_code;
                        // place ip to end of bytecode instruction (HALT)
                        vm.m_execution_contexts[0]->ip -= 4;
                    }
                    else
                    {
                        // reset ip if execution failed
                        vm.m_execution_contexts[0]->ip = m_old_ip;
                    }

                    state.reset();
                }
                else
                    std::cout << "Couldn't run code\n";
            }
        }

        return 0;
    }

    void Repl::cuiSetup()
    {
        m_repl.set_completion_callback(hookCompletion);
        m_repl.set_highlighter_callback(hookColor);
        m_repl.set_hint_callback(hookHint);

        m_repl.set_word_break_characters(" \t.,-%!;:=*~^'\"/?<>|[](){}");
        m_repl.set_completion_count_cutoff(128);
        m_repl.set_double_tab_completion(false);
        m_repl.set_complete_on_empty(true);
        m_repl.set_beep_on_ambiguous_completion(false);
        m_repl.set_no_color(false);
    }

    std::optional<std::string> Repl::getLine()
    {
        std::string prompt = fmt::format("main:{:0>3}> ", m_line_count);

        const char* buf { nullptr };
        do
        {
            buf = m_repl.input(prompt);
        } while ((buf == nullptr) && (errno == EAGAIN));
        std::string line = (buf != nullptr) ? std::string(buf) : "";

        // line history
        m_repl.history_add(line);
        trimWhitespace(line);

        // specific commands handling
        if (line == "(quit)" || buf == nullptr)
        {
            std::cout << "\nExiting REPL\n";
            m_running = false;
            return std::nullopt;
        }

        return line;
    }

    std::optional<std::string> Repl::getCodeBlock()
    {
        std::string code_block;
        long open_parentheses = 0;
        long open_braces = 0;

        while (m_running)
        {
            auto maybe_line = getLine();
            if (!maybe_line.has_value())
                return std::nullopt;
            else if (!maybe_line.value().empty())
            {
                code_block += maybe_line.value() + "\n";
                open_parentheses += countOpenEnclosures(maybe_line.value(), '(', ')');
                open_braces += countOpenEnclosures(maybe_line.value(), '{', '}');
            }

            // lines number incrementation
            ++m_line_count;
            if (open_parentheses == 0 && open_braces == 0)
                break;
        }

        return code_block;
    }
}
