
#ifndef ARK_COMPILER_EXECUTOR_HPP
#define ARK_COMPILER_EXECUTOR_HPP   

#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    class MacroExecutor {
        protected:
            MacroExecutor *m_next_executor;
            std::vector<std::unordered_map<std::string, Node>> m_macros;
        public:
            MacroExecutor () : m_next_executor(nullptr) {};
            MacroExecutor *set_next(MacroExecutor *executor);
            virtual void execute(std::vector<std::unordered_map<std::string, Node>> *macros) = 0;
    };

   
}

#endif