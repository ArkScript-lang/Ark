#ifndef ark_program
#define ark_program

#include <vector>
#include <iostream>
#include <string>

#include <Ark/Lang/Environment.hpp>
#include <Ark/Parser/Parser.hpp>
#include <Ark/Lang/Node.hpp>
#include <Ark/Function.hpp>

namespace Ark
{
    namespace Lang
    {
        class Program
        {
        public:
            Program();
            ~Program();

            void feed(const std::string& file);
            void execute(const Nodes& args={});

            void loadFunction(const std::string& name, Node::ProcType function);
            
            int operator[](const std::string& key) const;
            float operator[](const std::string& key) const;
            const std::string& operator[](const std::string& key) const;
            Function operator[](const std::string& key) const;

            friend std::ostream& operator<<(std::ostream& os, const Program& P);
        
        private:
            Node _execute(Node x, Environment* env);

            Ark::Parser::Parser m_parser;
            Environment m_global_env;
        };
    }
}

#endif  // ark_program
