
#ifndef ARK_COMPILER_EXECUTOR_HPP
#define ARK_COMPILER_EXECUTOR_HPP   

#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    class MacroExecutor {
        protected:
            unsigned int m_debug;
            MacroExecutor *m_next_executor = nullptr;
            std::vector<std::unordered_map<std::string, Node>> *m_macros = nullptr;
        public:
            MacroExecutor () : m_debug(0) {};
            MacroExecutor (unsigned int debug) : m_debug(debug) {};
            MacroExecutor *set_next(MacroExecutor *executor);
            virtual void execute(std::vector<std::unordered_map<std::string, Node>> *macros, Node &node) = 0;
            
        /**
         * @brief Find the nearest macro matching a giving name
         * 
         * @param name 
         * @return Node* nullptr if no macro was found
         */
        inline Node* find_nearest_macro(std::vector<std::unordered_map<std::string, Node>> *m_macros, const std::string& name)
        {
            if (m_macros->empty())
                return nullptr;

            for (auto it = m_macros->rbegin(); it != m_macros->rend(); ++it)
            {
                if (it->size() != 0)
                {
                    auto res = it->find(name);
                    if (res != it->end())
                        return &res->second;
                }
            }
            return nullptr;
        }
    };

   
}

#endif