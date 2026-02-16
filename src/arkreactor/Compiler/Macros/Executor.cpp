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

    void MacroExecutor::applyMacroProxy(Node& node, unsigned depth)
    {
        m_processor->applyMacro(node, depth);
    }

    void MacroExecutor::handleMacroNode(Node& node, const unsigned depth) const
    {
        m_processor->handleMacroNode(node, depth);
    }

    bool MacroExecutor::isTruthy(const Node& node) const
    {
        return m_processor->isTruthy(node);
    }

    Node MacroExecutor::evaluate(Node& node, const unsigned depth, const bool is_not_body) const
    {
        return m_processor->evaluate(node, depth, is_not_body);
    }

    void MacroExecutor::throwMacroProcessingError(const std::string& message, const Node& node)
    {
        m_processor->throwMacroProcessingError(message, node);
    }
}
