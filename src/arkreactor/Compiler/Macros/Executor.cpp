#include <Ark/Compiler/Macros/Executor.hpp>

#include <Ark/Compiler/Macros/Processor.hpp>

namespace Ark::internal
{
    MacroExecutor::MacroExecutor(MacroProcessor* processor, unsigned debug) :
        m_debug(debug),
        m_processor(processor)
    {}

    const Node* MacroExecutor::findNearestMacro(const std::string& name) const
    {
        return m_processor->findNearestMacro(name);
    }

    void MacroExecutor::registerMacro(Node& node)
    {
        m_processor->registerMacro(node);
    }

    bool MacroExecutor::isTruthy(const Node& node)
    {
        return m_processor->isTruthy(node);
    }

    Node MacroExecutor::evaluate(Node& node, bool is_not_body)
    {
        return m_processor->evaluate(node, is_not_body);
    }

    void MacroExecutor::unify(const std::unordered_map<std::string, Node>& map, Node& target, Node* parent)
    {
        m_processor->unify(map, target, parent);
    }

    void MacroExecutor::throwMacroProcessingError(const std::string& message, const Node& node)
    {
        m_processor->throwMacroProcessingError(message, node);
    }

    bool MacroExecutor::applyMacroProxy(Node& node)
    {
        return m_processor->applyMacro(node);
    }

    bool MacroExecutor::isPredefined(const std::string& symbol)
    {
        return m_processor->isPredefined(symbol);
    }
}
