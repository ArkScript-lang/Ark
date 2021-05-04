#ifndef ARK_COMPILER_EXECUTOR_HPP
#define ARK_COMPILER_EXECUTOR_HPP   

#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    class MacroProcessor;
    class MacroExecutor 
    {
        protected:
            unsigned int m_debug;
            static MacroProcessor *m_macroprocessor;

            Node* find_nearest_macro(const std::string& name);
            void registerMacro(Node &node);
            bool isTruthy(const Node& node);
            Node evaluate(Node& node, bool is_not_body);
            void apply_to(const std::unordered_map<std::string, Node>&, Node&, Node*);
            void throwMacroProcessingError(const std::string& message, const Node& node);
            void m_execute(Node& node);
        public:
            MacroExecutor (unsigned int debug = 0);
            static void init(MacroProcessor *macroprocessor);
            virtual void execute(Node &node) = 0;


    };

   
}

#endif