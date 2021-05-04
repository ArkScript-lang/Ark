#include <Ark/Compiler/MacroProcessor.hpp>
#include <Ark/Compiler/MacroExecutor.hpp>

namespace Ark::internal
{
    MacroProcessor* MacroExecutor::m_macroprocessor = nullptr;
    MacroExecutor::MacroExecutor(unsigned int debug) : m_debug(debug)
    {
        // macroprocessor pointer will be initialized in static init
    }

    void MacroExecutor::init(MacroProcessor* macroprocessor)
    {
        MacroExecutor::m_macroprocessor = macroprocessor;
    }

    Node* MacroExecutor::find_nearest_macro(const std::string& name)
    {
        return MacroExecutor::m_macroprocessor->find_nearest_macro(name);
    }

    void MacroExecutor::registerMacro(Node& node)
    {
        MacroExecutor::m_macroprocessor->registerMacro(node);
    }

    bool MacroExecutor::isTruthy(const Node& node)
    {
        return MacroExecutor::m_macroprocessor->isTruthy(node);
    }

    Node MacroExecutor::evaluate(Node& node, bool is_not_body)
    {
        return MacroExecutor::m_macroprocessor->evaluate(node, is_not_body);
    }
    void MacroExecutor::apply_to(const std::unordered_map<std::string, Node>& map, Node& target, Node* parent)
    {
        MacroExecutor::m_macroprocessor->apply_to(map, target, parent);
    }
    void MacroExecutor::throwMacroProcessingError(const std::string& message, const Node& node)
    {
        MacroExecutor::m_macroprocessor->throwMacroProcessingError(message, node);
    }
    void MacroExecutor::m_execute(Node& node)
    {
        MacroExecutor::m_macroprocessor->execute(node);
    }
}