#include <Ark/Compiler/MacroExecutors/SymbolExecutor.hpp>

namespace Ark::internal
{
    SymbolExecutor::SymbolExecutor(MacroProcessor* macroprocessor, unsigned debug) :
        MacroExecutor(macroprocessor, debug)
    {}

    bool SymbolExecutor::canHandle(Node& node)
    {
        return node.nodeType() == NodeType::Symbol;
    }

    bool SymbolExecutor::applyMacro(Node& node)
    {
        // error ?
        Node* macro = findNearestMacro(node.string());

        if (macro != nullptr)
        {
            if (m_debug >= 3)
                std::clog << "Found macro for " << node.string() << std::endl;

            // !{name value}
            if (macro->constList().size() == 2)
            {
                node = macro->list()[1];
                return true;
            }
        }

        return false;
    }
}
