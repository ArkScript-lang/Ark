#ifndef ark_program
#define ark_program

#include <vector>
#include <iostream>
#include <string>

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

            void add(const Node& node);
            // Iterator next();

            friend std::ostream& operator<<(std::ostream& os, const Program& P);
        
        private:
            std::vector<Node> m_program;
        };
    }
}

#endif  // ark_program