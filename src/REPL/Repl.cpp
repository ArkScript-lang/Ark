#include <functional>

#include <Ark/REPL/Repl.hpp>

namespace Ark
{
    Repl::Repl(uint16_t options, std::string lib_dir) :
        m_options(options), m_lib_dir(lib_dir)
    {}
    
    void Repl::run()
    {
        print_repl_header();

        cgui_setup();
        while(true)
        {
            Ark::State state(m_lib_dir, m_options);
            Ark::VM vm(&state);
            state.setDebug(0);

            std::stringstream code;
            int open_parentheses = 0;
            int open_braces = 0;

            while(true)
            {
                std::string infos = std::string(MAIN) + std::string(COLON) + std::to_string(m_lines) + std::string(COLON) + std::to_string(m_scope) + std::string(PROMPT);
                std::string line = m_repl.input(infos);

                // line history
                m_repl.history_add(line);
                trim_whitespace(line);

                // specific commands handling
                if(line.compare("(quit)") == 0 || line.compare("(q)") == 0)
                    return;

                // scopes
                scope_update(line);

                code << line << "\n";
                open_parentheses += count_open_parentheses(line);
                open_braces += count_open_braces(line);

                // lines number incrementation
                ++ m_lines;
                if(open_parentheses == 0 && open_braces == 0)
                    break;
            }

            // review necessary for main scope var re-use possible
            // and add returned value print 
            if(state.doString("{" + code.str() + "}"))
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
        std::cout << "Type \"(quit)\" or \"(q)\" to quit." << std::endl;
    }

    int Repl::count_open_parentheses(const std::string& line)
    {
        int open_parentheses = 0;

        for(const char& c: line)
        {
            switch(c)
            {
                case OPEN_PARENTHESE : ++ open_parentheses; break;
                case CLOSE_PARENTHESE : -- open_parentheses; break;
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
                case OPEN_BRACE : ++ open_braces; break;
                case CLOSE_BRACE : -- open_braces; break;
            }
        }

        return open_braces;
    }

    void Repl::trim_whitespace(std::string& line)
    {
        size_t string_begin = line.find_first_not_of(" \t");
        if(std::string::npos != string_begin)
        {
            size_t string_end = line.find_last_not_of(" \t");
            line = line.substr(string_begin, (string_end - string_begin + 1));
        }
    }

    void Repl::scope_update(const std::string& line)
    {
        if(line.find(OPEN_BRACE) != std::string::npos)
            ++ m_scope;
        else if(line.find(CLOSE_BRACE) != std::string::npos)
            if(m_scope != 0)
                -- m_scope;
    }

    void Repl::cgui_setup()
    {
        using namespace std::placeholders;

        m_repl.set_completion_callback(std::bind(&hook_completion, _1, _2, std::cref(KeywordsDict)));
        m_repl.set_highlighter_callback(std::bind(&hook_color, _1, _2, cref(ColorsRegexDict)));
        m_repl.set_hint_callback(std::bind(&hook_hint, _1, _2, _3, std::cref(KeywordsDict)));
    }
}