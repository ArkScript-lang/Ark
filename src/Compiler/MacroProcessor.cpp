#include <Ark/Compiler/MacroProcessor.hpp>

#include <Ark/Log.hpp>

#include <algorithm>

namespace Ark::internal
{
    MacroProcessor::MacroProcessor(unsigned debug, uint16_t options) noexcept :
        m_debug(debug), m_options(options)
    {}

    void MacroProcessor::feed(Node& ast)
    {
        // so that we can start with an empty scope
        m_macros.emplace_back();

        if (m_debug >= 2)
            Ark::logger.info("Processing macros...");

        // throwMacroProcessingError("oui", ast);

        // to be able to modify it
        m_ast = &ast;
        process(m_ast);

        if (m_debug >= 2)
            std::cout << (*m_ast) << std::endl;
    }

    Node& MacroProcessor::ast() noexcept
    {
        return *m_ast;
    }

    void MacroProcessor::registerMacro(Node* node)
    {
        // a macro needs at least 2 nodes, name + value is the minimal form
        if (node->const_list().size() < 2)
            throwMacroProcessingError("invalid macro, missing value", *node);

        Node *first_node = &node->list()[0],
             *second_node = &node->list()[1];

        // checks if argument list of !{name (args) body} are correct
        auto check_macro_args_list = [this](Node* args) {
            bool had_spread = false;
            for (const Node& n : args->const_list())
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
        if (node->const_list().size() == 2)
        {
            if (first_node->nodeType() == NodeType::Symbol)
            {
                m_macros.back()[first_node->string()] = *node;
                return;
            }
            throwMacroProcessingError("can not define a macro without a symbol", *first_node);
        }
        // !{name (args) body}
        else if (node->const_list().size() == 3 && first_node->nodeType() == NodeType::Symbol)
        {
            if (second_node->nodeType() != NodeType::List)
                throwMacroProcessingError("invalid macro argument's list", *second_node);
            else
            {
                check_macro_args_list(second_node);
                m_macros.back()[first_node->string()] = *node;
                return;
            }
        }
        // !{if cond then else}
        else if (std::size_t sz = node->const_list().size(); sz == 3 || sz == 4)
        {
            if (first_node->nodeType() == NodeType::Keyword && first_node->keyword() == Keyword::If)
            {
                execute(node);
                return;
            }
            else if (first_node->nodeType() == NodeType::Keyword)
                throwMacroProcessingError("the only authorized keyword in macros is `if'", *first_node);
        }
        // if we are here, it means we couldn't recognize the given macro, thus making it invalid
        throwMacroProcessingError("unrecognized macro form", *node);
    }

    void MacroProcessor::process(Node* node)
    {
        if (node->nodeType() == NodeType::List)
        {
            // create a scope only if needed
            if (!m_macros.back().empty())
                m_macros.emplace_back();

            // recursive call
            std::size_t i = 0;
            while (i < node->list().size())
            {
                if (node->list()[i].nodeType() == NodeType::Macro)
                {
                    registerMacro(&node->list()[i]);
                    node->list().erase(node->const_list().begin() + i);
                }
                else
                {
                    // execute only if we have registered macros
                    if ((m_macros.size() == 1 && m_macros[0].size() > 0) || m_macros.size() > 1)
                        execute(&node->list()[i]);

                    process(&node->list()[i]);

                    // go forward only if it isn't a macro, because we delete macros
                    // while running on the AST
                    ++i;
                }
            }

            // delete a scope only if needed
            if (!m_macros.back().empty())
                m_macros.pop_back();
        }
    }

    void MacroProcessor::execute(Node* node)
    {
        if (node->nodeType() == NodeType::Symbol)
        {
            Node* macro = find_nearest_macro(node->string());

            if (macro != nullptr)
            {
                if (m_debug >= 2)
                    Ark::logger.info("Found macro for", node->string());

                // !{name value}
                if (macro->const_list().size() == 2)
                {
                    *node = macro->list()[1];
                }
            }

            return;
        }
        else if (node->nodeType() == NodeType::Macro && node->list()[0].nodeType() == NodeType::Keyword)
        {
            Node* first = &node->list()[0];

            if (first->keyword() == Keyword::If)
            {
                Ark::logger.info("Found if macro");

                Node* cond = &node->list()[1];
                // evaluate cond
                if (Node temp = evaluate(cond); isTruthy(&temp))
                    *node = node->list()[2];
                else if (node->const_list().size() > 2)
                    *node = node->list()[3];
                else
                {
                    // remove node because nothing matched
                    node->list().clear();
                    node->setNodeType(NodeType::List);
                }
            }
        }
        else if (node->nodeType() == NodeType::List && node->const_list().size() > 0)
        {
            Node* first = &node->list()[0];
            Node* macro = find_nearest_macro(first->string());

            if (macro != nullptr)
            {
                if (m_debug >= 2)
                    Ark::logger.info("Found macro for", first->string());

                if (macro->const_list().size() == 2)
                    execute(first);
                // !{name (args) body}
                else if (macro->const_list().size() == 3)
                {
                    Node temp_body = macro->const_list()[2];
                    Node* args = &macro->list()[1];
                    // bind node->list() to temp_body using macro->const_list()[1]
                    std::unordered_map<std::string, Node> args_applied;
                    std::size_t j = 0;
                    for (std::size_t i = 1, end = node->const_list().size(); i < end; ++i)
                    {
                        const std::string& arg_name = args->list()[j].string();
                        if (args->list()[j].nodeType() == NodeType::Symbol)
                        {
                            args_applied[arg_name] = node->const_list()[i];
                            ++j;
                        }
                        else if (args->list()[j].nodeType() == NodeType::Spread)
                            // do not move j because we checked before that the spread is always the last one
                            args_applied[arg_name].push_back(node->const_list()[i]);
                    }
                    *node = evaluate(&temp_body);
                }
            }
        }
    }

    Node MacroProcessor::evaluate(Node* node)
    {
        if (node->nodeType() == NodeType::List && node->const_list().size() > 1)
        {
            const std::string& name = node->list()[0].string();
            if (Node* macro = find_nearest_macro(name); macro != nullptr)
            {
                execute(&node->list()[0]);
            }
            else if (name == "=")
            {}
            else if (name == "!=")
            {}
            else if (name == "<")
            {}
            else if (name == ">")
            {}
            else if (name == "<=")
            {}
            else if (name == ">=")
            {}
            else if (name == "not")
            {
                //return !isTruthy()
            }
            else if (name == "and")
            {}
            else if (name == "or")
            {}
            else if (name == "len")
            {}
            else if (name == "@")
            {}
            else if (name == "head")
            {}
            else if (name == "tail")
            {}
            else
                throwMacroProcessingError("", *node);
        }

        return *node;
    }

    bool MacroProcessor::isTruthy(Node* node)
    {
        if (node->nodeType() == NodeType::Symbol)
        {
            if (node->string() == "true")
                return true;
            else if (node->string() == "false" || node->string() == "nil")
                return false;
        }
        else if (node->nodeType() == NodeType::Number && node->number() != 0.0)
            return true;
        else if (node->nodeType() == NodeType::String && node->string().size() != 0)
            return true;
        else if (node->nodeType() == NodeType::Spread)
            throwMacroProcessingError("TODO une erreur", *node);
        return false;
    }
}