#include <Ark/REPL/Repl.hpp>

namespace Ark {
    void Repl::run()
    {
        const std::string new_prompt = ">> ";
        const std::string continuing_prompt = ".. ";

        print_repl_header();

        while(true)
        {
            std::stringstream code;
            int open_parentheses = 0;

            std::cout << new_prompt;

            for(std::string line; std::getline(std::cin, line);)
            {
                if (line.compare("(quit)") == 0)
                {
                    return;
                }

                code << line;

                open_parentheses += count_open_parentheses(line);

                if (open_parentheses == 0)
                {
                    break;
                }

                std::cout << continuing_prompt;
            }

            std::cout << code.str() << "\n";
        }
    }

    void Repl::print_repl_header()
    {
        std::cout << "ArkScript REPL -- ";
        std::cout << "Version " << ARK_VERSION_MAJOR << "." << ARK_VERSION_MINOR << "." << ARK_VERSION_PATCH << " ";
        std::cout << "[LICENSE: Mozilla Public License 2.0]\n";
        std::cout << "Type \"(quit)\" to quit.\n";
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
}