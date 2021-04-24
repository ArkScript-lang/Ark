#include <Ark/Compiler/MacroExecutors/MacroExecutorPipeline.hpp>

namespace Ark::internal
{
    MacroExecutorPipeline::MacroExecutorPipeline(
        std::function<Node *(const std::string &name)> find_nearest_macro,
        std::function<void(Node &node)> registerMacro,
        std::function<bool(const Node &node)> isTruthy,
        std::function<Node(Node &node, bool is_not_body)> evaluate,
        std::function<void(const std::unordered_map<std::string, Node> &, Node &, Node *)> apply_to,
        std::function<void(const std::string &message, const Node &node)> throwMacroProcessingError,
        std::function<void(Node &node)> func_execute)
    {
        this->find_nearest_macro = find_nearest_macro;
        this->registerMacro = registerMacro;
        this->isTruthy = isTruthy;
        this->evaluate = evaluate;
        this->apply_to = apply_to;
        this->throwMacroProcessingError = throwMacroProcessingError;
        this->func_execute = func_execute;

        this->m_executors = {
            new SymbolExecutor(),
            new ConditionalExecutor(),
            new ListExecutor()
        };
    }
    void MacroExecutorPipeline::execute(Node &node)
    {
        for (MacroExecutor *executor : m_executors)
        {
            executor->execute(
                find_nearest_macro,
                registerMacro,
                isTruthy,
                evaluate,
                apply_to,
                throwMacroProcessingError,
                func_execute,
                node);
        }
    }
}