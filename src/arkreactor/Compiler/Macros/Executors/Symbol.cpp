#include <Ark/Compiler/Macros/Executors/Symbol.hpp>

namespace Ark::internal
{
    bool SymbolExecutor::canHandle(Node& node)
    {
        return node.nodeType() == NodeType::Symbol;
    }

    bool SymbolExecutor::applyMacro(Node& node)
    {
        if (const Node* macro = findNearestMacro(node.string()); macro != nullptr)
        {
            // ($ name value)
            if (macro->constList().size() == 2)
            {
                node = macro->constList()[1];
                return true;
            }
        }

        return false;
    }
}
