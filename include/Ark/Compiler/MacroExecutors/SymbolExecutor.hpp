#ifndef ARK_COMPILER_SYMBOLEXECUTOR_HPP
#define ARK_COMPILER_SYMBOLEXECUTOR_HPP   
#include <Ark/Compiler/MacroExecutor.hpp>
#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>
namespace Ark::internal {
    class SymbolExecutor : public MacroExecutor{
        public:
            void execute(std::vector<std::unordered_map<std::string, Node>> *macros, Node &node);
    };

}

#endif