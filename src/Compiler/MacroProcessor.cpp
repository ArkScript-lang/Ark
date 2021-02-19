#include <Ark/Compiler/MacroProcessor.hpp>

#include <Ark/Log.hpp>

#include <algorithm>

namespace Ark::internal
{
    MacroProcessor::MacroProcessor(unsigned debug, uint16_t options) noexcept :
        m_debug(debug), m_options(options)
    {}

    void MacroProcessor::feed(const Node& ast)
    {
        // so that we can start with an empty scope
        m_macros.emplace_back();

        // to be able to modify it
        m_ast = ast;
        process(m_ast);
    }

    const Node& MacroProcessor::ast() const noexcept
    {
        return m_ast;
    }

    void MacroProcessor::registerMacro(const Node& node)
    {
        // a macro needs at least 2 nodes, name + value is the minimal form
        if (node.const_list().size() < 2)
            throwMacroProcessingError("invalid macro, missing value", node);

        const Node& first_node = node.const_list()[0],
                    second_node = node.const_list()[1];

        // checks if argument list of !{name (args) body} are correct
        auto check_macro_args_list = [this](const Node& args) {
            bool had_spread = false;
            for (const Node& n : args.const_list())
            {
                if (n.nodeType() != NodeType::Symbol && n.nodeType() != NodeType::Spread)
                    throwMacroProcessingError("invalid macro argument's list, expected symbols", n);
                else if (n.nodeType() == NodeType::Spread)
                {
                    if (had_spread)
                        throwMacroProcessingError("got another spread argument, only one is allowed", n);
                    had_spread = true;
                }
            }
        };

        // !{name value}
        if (node.const_list().size() == 2)
        {
            if (first_node.nodeType() == NodeType::Symbol)
            {
                m_macros.back()[first_node.string()] = node;
                return;
            }
            throwMacroProcessingError("can not define a macro without a symbol", first_node);
        }
        // !{name (args) body}
        else if (node.const_list().size() == 3 && first_node.nodeType() == NodeType::Symbol)
        {
            if (second_node.nodeType() != NodeType::List)
                throwMacroProcessingError("invalid macro argument's list", second_node);
            else
            {
                check_macro_args_list(second_node);
                m_macros.back()[first_node.string()] = node;
                return;
            }
        }
        // !{if cond then else}
        else if (std::size_t sz = node.const_list().size(); sz == 3 || sz == 4)
        {
            if (first_node.nodeType() == NodeType::Keyword && first_node.keyword() == Keyword::If)
            {
                // TODO execute if
                return;
            }
            else if (first_node.nodeType() == NodeType::Keyword)
                throwMacroProcessingError("the only authorized keyword in macros is `if'", first_node);
        }
        // if we are here, it means we couldn't recognize the given macro, thus making it invalid
        throwMacroProcessingError("unrecognized macro form", node);
    }

    void MacroProcessor::process(Node& node)
    {
        if (node.nodeType() == NodeType::List)
        {
            m_macros.emplace_back();
            // recursive call
            std::size_t i = 0;
            while (i < node.list().size())
            {
                Node sub = node.list()[i];
                if (sub.nodeType() == NodeType::Macro)
                {
                    registerMacro(sub);
                    node.list().erase(node.const_list().begin() + i);
                }
                else
                {
                    process(sub);
                    // go forward only if it isn't a macro, because we delete macros
                    // while running on the AST
                    ++i;
                }
            }
            m_macros.pop_back();
        }
        // TODO identify where macros are used
    }
}