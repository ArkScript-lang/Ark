#ifndef ARK_COMPILER_LISTEXECUTOR_HPP
#define ARK_COMPILER_LISTEXECUTOR_HPP   
#include <Ark/Compiler/MacroExecutor.hpp>
#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal 
{
    class ListExecutor : public MacroExecutor
    {
        public:
            void execute(Node &node);
    };

}

#endif