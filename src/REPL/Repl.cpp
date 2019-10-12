#include <Ark/REPL/Repl.hpp>

namespace Ark {
    void Repl::run()
    {
        const std::string new_prompt = ">> ";
        const std::string continuing_prompt = ".. ";

        print_repl_header();

        while(true)
        {
            Ark::Compiler compiler(false);
            Ark::State state(m_lib_dir);
            Ark::VM vm(&state, m_options);
            state.setDebug(false);

            std::stringstream code;
            int open_parentheses = 0;
            int open_braces = 0;
            bool new_command = true;

            std::cout << new_prompt;
            for(std::string line; std::getline(std::cin, line);)
            {
                if (line.length() == 0)
                {
                    std::cout << (new_command ? new_prompt : continuing_prompt);
                    continue;
                }

                if (line.compare("(quit)") == 0)
                {
                    return;
                }

                code << line << "\n";
                open_parentheses += count_open_parentheses(line);
                open_braces += count_open_braces(line);

                if (open_parentheses == 0 && open_braces == 0)
                {
                    break;
                }

                new_command = false;
                std::cout << continuing_prompt;
            }

            try
            {
                compiler.feed("{" + code.str() + "}");
                compiler.compile();

                state.feed(compiler.bytecode());
                vm.run();
            }
            catch (const std::runtime_error& e) {
                std::cerr << e.what() << std::endl;
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown lexer-parser-or-compiler error" << std::endl;
            }
        }
    }

    inline void Repl::print_repl_header()
    {
        std::cout << "ArkScript REPL -- ";
        std::cout << "Version " << ARK_VERSION_MAJOR << "." << ARK_VERSION_MINOR << "." << ARK_VERSION_PATCH << " ";
        std::cout << "[LICENSE: Mozilla Public License 2.0]" << std::endl;
        std::cout << "Type \"(quit)\" to quit." << std::endl;
    }

    int Repl::count_open_parentheses(std::string line)
    {
        int open_parentheses = 0;

        for(char& c: line)
        {
            switch(c)
            {
                case '(': open_parentheses++; break;
                case ')': open_parentheses--; break;
            }
        }

        return open_parentheses;
    }

    int Repl::count_open_braces(std::string line)
    {
        int open_braces = 0;

        for(char& c: line)
        {
            switch(c)
            {
                case '{': open_braces++; break;
                case '}': open_braces--; break;
            }
        }

        return open_braces;
    }
}