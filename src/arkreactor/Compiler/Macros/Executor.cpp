#include <Ark/Compiler/Macros/Executor.hpp>

#include <Ark/Compiler/Macros/Processor.hpp>

namespace Ark::internal
{
    MacroExecutor::MacroExecutor(MacroProcessor* processor, unsigned debug) :
        m_debug(debug),
        m_processor(processor)
    {}

    void MacroExecutor::setWithFileAttributes(const Node origin, Node& output, const Node& macro)
    {
        output = macro;
        output.setFilename(origin.filename());
        output.setPos(origin.line(), origin.col());
    }

    const Node* MacroExecutor::findNearestMacro(const std::string& name) const
    {
        return m_processor->findNearestMacro(name);
    }

    void MacroExecutor::registerMacro(Node& node) const
    {
        m_processor->registerMacro(node);
    }

    bool MacroExecutor::isTruthy(const Node& node) const
    {
        return m_processor->isTruthy(node);
    }

    Node MacroExecutor::evaluate(Node& node, const bool is_not_body) const
    {
        return m_processor->evaluate(node, is_not_body);
    }

    void MacroExecutor::unify(const std::unordered_map<std::string, Node>& map, Node& target, Node* parent) const
    {
        m_processor->unify(map, target, parent, /* index= */ 0, /* unify_depth= */ 0);
    }

    void MacroExecutor::throwMacroProcessingError(const std::string& message, const Node& node) const
    {
        m_processor->throwMacroProcessingError(message, node);
    }

    bool MacroExecutor::applyMacroProxy(Node& node) const
    {
        return m_processor->applyMacro(node);
    }

    bool MacroExecutor::isPredefined(const std::string& symbol) const
    {
        return m_processor->isPredefined(symbol);
    }
}
