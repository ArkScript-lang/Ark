#include <Ark/Compiler/MacroProcessor.hpp>

#include <Ark/Log.hpp>

namespace Ark::internal
{
    MacroProcessor::MacroProcessor(unsigned debug, uint16_t options) noexcept :
        m_debug(debug), m_options(options)
    {}

    void MacroProcessor::feed(const Node& ast)
    {
        process(ast);
    }

    const Node& MacroProcessor::ast() const noexcept
    {
        return m_ast;
    }

    void MacroProcessor::process(const Node& node)
    {
        if (node.nodeType() == NodeType::Macro)
        {
            // !{name value}
            if (node.const_list().size() == 2)
            {}
            // !{name (args) body}
            else if (node.const_list().size() == 3)
            {}
            // !{if cond then else}
            else if (std::size_t sz = node.const_list().size(); sz == 3 || sz == 4)
            {}
            else
                throwMacroProcessingError("error message", node);
        }
        else if (node.nodeType() == NodeType::List)
        {}
        else
        {}
    }
}