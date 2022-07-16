#include <Ark/Compiler/JsonCompiler.hpp>

#include <Ark/Compiler/Macros/Processor.hpp>

#include <utility>
#include <exception>
#include <stdexcept>
#include <string>

namespace Ark
{
    using namespace internal;

    JsonCompiler::JsonCompiler(unsigned debug, const std::vector<std::string>& libenv, uint16_t options) :
        m_parser(debug, options, libenv), m_optimizer(options),
        m_options(options), m_debug(debug)
    {}

    void JsonCompiler::feed(const std::string& code, const std::string& filename)
    {
        m_parser.feed(code, filename);

        MacroProcessor mp(m_debug, m_options);
        mp.feed(m_parser.ast());
        m_optimizer.feed(mp.ast());
    }

    std::string JsonCompiler::compile()
    {
        return _compile(m_optimizer.ast());
    }

    template <typename... Args>
    std::string string_format(const std::string& format, Args&&... args)
    {
        constexpr size_t buffer_size = 8192;
        static char buf[buffer_size] = { 0 };
        std::string to_return = "";
        while (snprintf(buf, buffer_size - 1, format.c_str(), std::forward<Args>(args)...) == buffer_size - 1)
            to_return += std::string(buf);
        to_return += std::string(buf);
        return to_return;
    }

    std::string JsonCompiler::_compile(const Node& node)
    {
        std::string json = "";

        switch (node.nodeType())
        {
            case NodeType::Symbol:
            {
                json += string_format(
                    R"({"type": "Symbol", "name": "%s"})",
                    node.string().c_str());
                break;
            }

            case NodeType::Capture:
            {
                json += string_format(
                    R"({"type": "Capture", "name": "%s"})",
                    node.string().c_str());
                break;
            }

            case NodeType::GetField:
            {
                json += string_format(
                    R"({"type": "GetField", "name": "%s"})",
                    node.string().c_str());
                break;
            }

            case NodeType::String:
            {
                json += string_format(
                    R"({"type": "String", "value": "%s"})",
                    node.string().c_str());
                break;
            }

            case NodeType::Number:
            {
                json += string_format(
                    R"({"type": "Number", "value": %f})",
                    node.number());
                break;
            }

            case NodeType::List:
            {
                if (node.constList().size() > 1 && node.constList()[0].nodeType() == NodeType::Keyword)
                {
                    Node keyword = node.constList()[0];
                    switch (keyword.keyword())
                    {
                        case Keyword::Fun:
                        {
                            // (fun (args) (body))
                            std::string args = "";
                            Node args_node = node.constList()[1];
                            for (std::size_t i = 0, end = args_node.constList().size(); i < end; ++i)
                            {
                                args += _compile(args_node.constList()[i]);
                                if (end > 1 && i != end - 1)
                                    args += ", ";
                            }

                            json += string_format(
                                R"({"type": "Fun", "args": [%s], "body": %s})",
                                args.c_str(), _compile(node.constList()[2]).c_str());
                            break;
                        }

                        case Keyword::Let:
                        {
                            // (let name value)
                            json += string_format(
                                R"({"type": "Let", "name": %s, "value": %s})",
                                _compile(node.constList()[1]).c_str(), _compile(node.constList()[2]).c_str());
                            break;
                        }

                        case Keyword::Mut:
                        {
                            // (mut name value)
                            json += string_format(
                                R"({"type": "Mut", "name": %s, "value": %s})",
                                _compile(node.constList()[1]).c_str(), _compile(node.constList()[2]).c_str());
                            break;
                        }

                        case Keyword::Set:
                        {
                            // (set name value)
                            json += string_format(
                                R"({"type": "Set", "name": %s, "value": %s})",
                                _compile(node.constList()[1]).c_str(), _compile(node.constList()[2]).c_str());
                            break;
                        }

                        case Keyword::If:
                        {
                            // (if condition then else)
                            json += string_format(
                                R"({"type": "If", "condition": %s, "then": %s, "else": %s})",
                                _compile(node.constList()[1]).c_str(), _compile(node.constList()[2]).c_str(), _compile(node.constList()[3]).c_str());
                            break;
                        }

                        case Keyword::While:
                        {
                            // (while condition body)
                            json += string_format(
                                R"({"type": "While", "condition": %s, "body": %s})",
                                _compile(node.constList()[1]).c_str(), _compile(node.constList()[2]).c_str());
                            break;
                        }

                        case Keyword::Begin:
                        {
                            // (begin body)
                            json += R"({"type": "Begin", "children": )";
                            json += toJsonList(node, 1) + "}";
                            break;
                        }

                        case Keyword::Import:
                        {
                            // (import value)
                            json += string_format(
                                R"({"type": "Import", "value": %s})",
                                _compile(node.constList()[1]).c_str());
                            break;
                        }

                        case Keyword::Quote:
                        {
                            // (quote value)
                            json += string_format(
                                R"({"type": "Quote", "value": %s})",
                                _compile(node.constList()[1]).c_str());
                            break;
                        }

                        case Keyword::Del:
                        {
                            // (del value)
                            json += string_format(
                                R"({"type": "Del", "value": %s})",
                                _compile(node.constList()[1]).c_str());
                            break;
                        }
                    }
                }
                else if (node.constList().size() > 1 && node.constList()[0].nodeType() == NodeType::Symbol)
                {
                    // (foo bar 1)
                    json += string_format(
                        R"({"type": "FunctionCall", "name": %s, "args": )",
                        _compile(node.constList()[0]).c_str());
                    json += toJsonList(node, 1) + "}";
                }
                else
                    json += toJsonList(node, 0);

                break;
            }

            default:
                throw std::runtime_error(string_format(
                    "Not handled NodeType::%s (%s at %zu:%zu), please report this error on GitHub",
                    nodeTypes[static_cast<std::size_t>(node.nodeType())].data(),
                    node.filename().c_str(),
                    node.line(),
                    node.col()));
        }
        return json;
    }

    std::string JsonCompiler::toJsonList(const Node& node, std::size_t start)
    {
        std::string json = "[";
        for (std::size_t i = start, end = node.constList().size(); i < end; ++i)
        {
            json += _compile(node.constList()[i]);
            if (i != end - 1)
                json += ", ";
        }
        json += "]";
        return json;
    }
}
