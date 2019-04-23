#ifndef ark_program
#define ark_program

#include <vector>
#include <iostream>
#include <string>

#include <Ark/Parser/Environment.hpp>
#include <Ark/Parser/Parser.hpp>
#include <Ark/Parser/Node.hpp>

namespace Ark
{
    namespace Parser
    {
        class Program
        {
        public:
            Program();
            ~Program();

            void feed(const std::string& file);
            void execute(const Nodes& args={});

            void loadFunction(const std::string& name, Node::ProcType function);

            friend std::ostream& operator<<(std::ostream& os, const Program& P);
        
        private:
            Node _execute(Node x, Environment* env);

            Parser m_parser;
            Environment m_global_env;
        };
    }
}

#endif  // ark_program
