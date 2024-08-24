#include <fstream>
#include <iostream>
#include <filesystem>
#include <fmt/core.h>
#include <ranges>

#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Compiler/Common.hpp>

#include <CLI/REPL/Repl.hpp>
#include <CLI/REPL/Utils.hpp>

namespace Ark
{
    using namespace internal;
    using namespace replxx;

    Repl::Repl(const std::vector<std::filesystem::path>& lib_env) :
        m_line_count(1), m_running(true),
        m_old_ip(0), m_lib_env(lib_env),
        m_state(m_lib_env), m_vm(m_state), m_has_init_vm(false)
    {
        m_keywords.reserve(keywords.size() + listInstructions.size() + operators.size() + Builtins::builtins.size());
        for (auto keyword : keywords)
            m_keywords.emplace_back(keyword);
        for (auto inst : listInstructions)
            m_keywords.emplace_back(inst);
        for (auto op : operators)
            m_keywords.emplace_back(op);
        for (const auto& builtin : std::ranges::views::keys(Builtins::builtins))
            m_keywords.push_back(builtin);

        m_words_colors.reserve(keywords.size() + listInstructions.size() + operators.size() + Builtins::builtins.size() + 2);
        for (auto keyword : keywords)
            m_words_colors.emplace_back(keyword, Replxx::Color::BRIGHTRED);
        for (auto inst : listInstructions)
            m_words_colors.emplace_back(inst, Replxx::Color::GREEN);
        for (auto op : operators)
        {
            auto safe_op = std::string(op);
            if (const auto it = safe_op.find_first_of(R"(-+=/*<>[]()?")"); it != std::string::npos)
                safe_op.insert(it, "\\");
            m_words_colors.emplace_back(safe_op, Replxx::Color::BRIGHTBLUE);
        }
        for (const auto& builtin : std::ranges::views::keys(Builtins::builtins))
            m_words_colors.emplace_back(builtin, Replxx::Color::GREEN);

        m_words_colors.emplace_back("[\\-|+]?[0-9]+(\\.[0-9]+)?", Replxx::Color::YELLOW);
        m_words_colors.emplace_back("\".*\"", Replxx::Color::MAGENTA);
    }

    int Repl::run()
    {
        fmt::print("ArkScript REPL -- Version {} [LICENSE: Mozilla Public License 2.0]\nType \"quit\" to quit. Try \"help\" for more information\n", ARK_FULL_VERSION);
        cuiSetup();

        while (m_running)
        {
            auto maybe_block = getCodeBlock();

            // save a valid ip if execution failed
            m_old_ip = m_vm.m_execution_contexts[0]->ip;
            if (maybe_block.has_value() && !maybe_block.value().empty())
            {
                std::string new_code = m_code + maybe_block.value();
                if (m_state.doString(new_code))
                {
                    // for only one vm init
                    if (!m_has_init_vm)
                    {
                        m_vm.init();
                        m_has_init_vm = true;
                    }
                    else
                        std::ignore = m_vm.forceReloadPlugins();

                    if (m_vm.safeRun(*m_vm.m_execution_contexts[0]) == 0)
                    {
                        // save good code
                        m_code = new_code;
                        // place ip to end of bytecode instruction (HALT)
                        m_vm.m_execution_contexts[0]->ip -= 4;
                    }
                    else
                    {
                        // reset ip if execution failed
                        m_vm.m_execution_contexts[0]->ip = m_old_ip;
                    }

                    m_state.reset();
                }
                else
                    std::cout << "\nCouldn't run code\n";
            }
        }

        return 0;
    }

    void Repl::cuiSetup()
    {
        m_repl.set_completion_callback([this](const std::string& ctx, int& len) {
            return hookCompletion(m_keywords, ctx, len);
        });
        m_repl.set_highlighter_callback([this](const std::string& ctx, Replxx::colors_t& colors) {
            return hookColor(m_words_colors, ctx, colors);
        });
        m_repl.set_hint_callback([this](const std::string& ctx, int& len, Replxx::Color& color) {
            return hookHint(m_keywords, ctx, len, color);
        });

        m_repl.set_word_break_characters(" \t.,-%!;:=*~^'\"/?<>|[](){}");
        m_repl.set_completion_count_cutoff(128);
        m_repl.set_double_tab_completion(true);
        m_repl.set_complete_on_empty(true);
        m_repl.set_beep_on_ambiguous_completion(false);
        m_repl.set_no_color(false);

        m_repl.bind_key_internal(Replxx::KEY::HOME, "move_cursor_to_begining_of_line");
        m_repl.bind_key_internal(Replxx::KEY::END, "move_cursor_to_end_of_line");
        m_repl.bind_key_internal(Replxx::KEY::TAB, "complete_line");
        m_repl.bind_key_internal(Replxx::KEY::control(Replxx::KEY::LEFT), "move_cursor_one_word_left");
        m_repl.bind_key_internal(Replxx::KEY::control(Replxx::KEY::RIGHT), "move_cursor_one_word_right");
        m_repl.bind_key_internal(Replxx::KEY::control(Replxx::KEY::UP), "hint_previous");
        m_repl.bind_key_internal(Replxx::KEY::control(Replxx::KEY::DOWN), "hint_next");
        m_repl.bind_key_internal(Replxx::KEY::control(Replxx::KEY::ENTER), "commit_line");
        m_repl.bind_key_internal(Replxx::KEY::control('R'), "history_incremental_search");
        m_repl.bind_key_internal(Replxx::KEY::control('W'), "kill_to_begining_of_word");
        m_repl.bind_key_internal(Replxx::KEY::control('U'), "kill_to_begining_of_line");
        m_repl.bind_key_internal(Replxx::KEY::control('K'), "kill_to_end_of_line");
        m_repl.bind_key_internal(Replxx::KEY::control('Y'), "yank");
        m_repl.bind_key_internal(Replxx::KEY::control('L'), "clear_screen");
        m_repl.bind_key_internal(Replxx::KEY::control('D'), "send_eof");
        m_repl.bind_key_internal(Replxx::KEY::control('C'), "abort_line");
        m_repl.bind_key_internal(Replxx::KEY::control('T'), "transpose_characters");
    }

    std::optional<std::string> Repl::getLine(const bool continuation)
    {
        const std::string prompt = fmt::format("main:{:0>3}{} ", m_line_count, continuation ? ":" : ">");

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
        if (line == "quit" || buf == nullptr)
        {
            std::cout << "\nExiting REPL\n";
            m_running = false;

            return std::nullopt;
        }
        if (line == "help")
        {
            std::cout << "Available commands:\n";
            std::cout << "  help -- display this message\n";
            std::cout << "  quit -- quit the REPL\n";
            std::cout << "  save -- save the history to disk\n";
            std::cout << "  history -- print saved code\n";
            std::cout << "  reset -- reset the VM state\n";

            return std::nullopt;
        }
        if (line == "save")
        {
            std::ofstream history_file("arkscript_repl_history.ark");
            m_repl.history_save(history_file);

            fmt::print("Saved {} lines of history to arkscript_repl_history.ark\n", m_line_count);

            return std::nullopt;
        }
        if (line == "history")
        {
            std::cout << "\n"
                      << m_code << "\n";

            return std::nullopt;
        }
        if (line == "reset")
        {
            m_state.reset();
            m_has_init_vm = false;
            m_code.clear();

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
            const bool unfinished_block = open_parentheses != 0 || open_braces != 0;

            auto maybe_line = getLine(unfinished_block);
            if (!maybe_line.has_value() && !unfinished_block)
                return std::nullopt;

            if (maybe_line.has_value() && !maybe_line.value().empty())
            {
                code_block += maybe_line.value() + "\n";
                open_parentheses += countOpenEnclosures(maybe_line.value(), '(', ')');
                open_braces += countOpenEnclosures(maybe_line.value(), '{', '}');

                // lines number incrementation
                ++m_line_count;
                if (open_parentheses == 0 && open_braces == 0)
                    break;
            }
        }

        return code_block;
    }
}
