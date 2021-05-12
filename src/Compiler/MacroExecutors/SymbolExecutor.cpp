#include <Ark/Compiler/MacroExecutors/SymbolExecutor.hpp>
#include <Ark/Log.hpp>

namespace Ark::internal
{
    SymbolExecutor::SymbolExecutor(MacroProcessor* macroprocessor, int debug) : MacroExecutor(macroprocessor, debug)
    {

    }

    bool SymbolExecutor::m_canHandle(Node& node)
    {
        return node.nodeType() == NodeType::Symbol;
    }
    
    void SymbolExecutor::m_execute(Node& node)
    {
        // error ?
        Node* macro = m_find_nearest_macro(node.string());

        if (macro != nullptr)
        {
            if (m_debug >= 3)
                Ark::logger.info("Found macro for", node.string());

            // !{name value}
            if (macro->const_list().size() == 2)
                node = macro->list()[1];
        }
    }
}
