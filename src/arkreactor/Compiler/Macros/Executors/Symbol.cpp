#include <Ark/Compiler/Macros/Executors/Symbol.hpp>

namespace Ark::internal
{
    bool SymbolExecutor::canHandle(Node& node)
    {
        return node.nodeType() == NodeType::Symbol;
    }

    bool SymbolExecutor::applyMacro(Node& node, const unsigned depth)
    {
        if (const Node* macro = findNearestMacro(node.string()); macro != nullptr)
        {
            // ($ name value)
            if (macro->constList().size() == 2)
            {
                node.updateValueAndType(macro->constList()[1]);
                node = evaluate(node, depth + 1, false);
                applyMacroProxy(node, depth + 1);
                return true;
            }
        }

        return false;
    }
}
