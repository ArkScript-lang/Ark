#include <Ark/REPL/Repl.hpp>

namespace Ark
{
    Repl::Repl(uint16_t options, std::string lib_dir) :
        m_options(options), m_lib_dir(lib_dir)
    {}
    
    void Repl::run()
    {
        const std::string new_prompt = ">> ";
        const std::string continuing_prompt = ".. ";

        print_repl_header();

        while (true)
        {
            Ark::State state(m_lib_dir, m_options);
            Ark::VM vm(&state);
            state.setDebug(0);

            std::stringstream code;
            int open_parentheses = 0;
            int open_braces = 0;
            bool new_command = true;

            std::cout << new_prompt;
            for(std::string line; std::getline(std::cin, line);)
            {
                trim_whitespace(line);

                // empty lines and comments handling
                if (line.length() == 0 || line.at(0) == '#')
                {
                    std::cout << (new_command ? new_prompt : continuing_prompt);
                    continue;
                }

                // specific commands handling
                if (line.compare("(quit)") == 0)
                    return;

                code << line << "\n";
                open_parentheses += count_open_parentheses(line);
                open_braces += count_open_braces(line);

                if (open_parentheses == 0 && open_braces == 0)
                    break;

                new_command = false;
                std::cout << continuing_prompt;
            }

            if (std::cin.eof())
                return;

            if (state.doString("{" + code.str() + "}"))
                vm.run();
            else
                std::cerr << "Ark::State::doString failed" << std::endl;
        }
    }

    inline void Repl::print_repl_header()
    {
        std::cout << "ArkScript REPL -- ";
        std::cout << "Version " << ARK_VERSION_MAJOR << "." << ARK_VERSION_MINOR << "." << ARK_VERSION_PATCH << " ";
        std::cout << "[LICENSE: Mozilla Public License 2.0]" << std::endl;
        std::cout << "Type \"(quit)\" to quit." << std::endl;
    }

    int Repl::count_open_parentheses(const std::string& line)
    {
        int open_parentheses = 0;

        for(const char& c: line)
        {
            switch(c)
            {
                case '(': open_parentheses++; break;
                case ')': open_parentheses--; break;
            }
        }

        return open_parentheses;
    }

    int Repl::count_open_braces(const std::string& line)
    {
        int open_braces = 0;

        for(const char& c: line)
        {
            switch(c)
            {
                case '{': open_braces++; break;
                case '}': open_braces--; break;
            }
        }

        return open_braces;
    }

    void Repl::trim_whitespace(std::string& line)
    {
        size_t string_begin = line.find_first_not_of(" \t");
        if (std::string::npos != string_begin)
        {
            size_t string_end = line.find_last_not_of(" \t");
            line = line.substr(string_begin, (string_end - string_begin + 1));
        }
    }
}