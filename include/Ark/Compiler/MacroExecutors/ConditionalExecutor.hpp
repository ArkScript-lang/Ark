/**
 * @file ConditionalExecutor.hpp
 * @author Ray John Alovera (rakista112@gmail.com)
 * @brief Executor for Conditional Macros
 * @version 0.1
 * @date 2021-05-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARK_COMPILER_CONDITIONALEXECUTOR_HPP
#define ARK_COMPILER_CONDITIONALEXECUTOR_HPP
#include <Ark/Compiler/MacroExecutor.hpp>
#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief Handles Conditional macros
     * 
    */
    class ConditionalExecutor : public MacroExecutor
    {
    public:
        void execute(Node &node);
    };

}

#endif