#include <CLI/JsonCompiler.hpp>

#include <string>
#include <vector>
#include <ranges>
#include <Ark/Exceptions.hpp>

#include <fmt/core.h>

using namespace Ark::internal;

JsonCompiler::JsonCompiler(unsigned debug, const std::vector<std::filesystem::path>& lib_env) :
    m_welder(debug, lib_env, Ark::DefaultFeatures & ~Ark::FeatureImportSolver)
{}

void JsonCompiler::feed(const std::string& filename)
{
    m_welder.computeASTFromFile(filename);
}

std::string JsonCompiler::compile()
{
    return _compile(m_welder.ast());
}

std::string JsonCompiler::_compile(const Node& node)
{
    std::string json;

    switch (node.nodeType())
    {
        case NodeType::Symbol:
        {
            json += fmt::format(
                R"({{"type": "Symbol", "name": "{}"}})",
                node.string().c_str());
            break;
        }

        case NodeType::Capture:
        {
            json += fmt::format(
                R"({{"type": "Capture", "name": "{}"}})",
                node.string().c_str());
            break;
        }

        case NodeType::Field:
        {
            json += R"({"type": "Field", "children": )";
            json += toJsonList(node, 0) + "}";
            break;
        }

        case NodeType::String:
        {
            json += fmt::format(
                R"({{"type": "String", "value": "{}"}})",
                node.string().c_str());
            break;
        }

        case NodeType::Number:
        {
            json += fmt::format(
                R"({{"type": "Number", "value": {}}})",
                node.number());
            break;
        }

        case NodeType::List:
        {
            if (!node.constList().empty() && node.constList()[0].nodeType() == NodeType::Keyword)
            {
                Node keyword = node.constList()[0];
                switch (keyword.keyword())
                {
                    case Keyword::Fun:
                    {
                        // (fun (args) (body))
                        std::string args;
                        Node args_node = node.constList()[1];
                        for (std::size_t i = 0, end = args_node.constList().size(); i < end; ++i)
                        {
                            args += _compile(args_node.constList()[i]);
                            if (end > 1 && i != end - 1)
                                args += ", ";
                        }

                        json += fmt::format(
                            R"({{"type": "Fun", "args": [{}], "body": {}}})",
                            args.c_str(), _compile(node.constList()[2]).c_str());
                        break;
                    }

                    case Keyword::Let:
                    {
                        // (let name value)
                        json += fmt::format(
                            R"({{"type": "Let", "name": {}, "value": {}}})",
                            _compile(node.constList()[1]).c_str(), _compile(node.constList()[2]).c_str());
                        break;
                    }

                    case Keyword::Mut:
                    {
                        // (mut name value)
                        json += fmt::format(
                            R"({{"type": "Mut", "name": {}, "value": {}}})",
                            _compile(node.constList()[1]).c_str(), _compile(node.constList()[2]).c_str());
                        break;
                    }

                    case Keyword::Set:
                    {
                        // (set name value)
                        json += fmt::format(
                            R"({{"type": "Set", "name": {}, "value": {}}})",
                            _compile(node.constList()[1]).c_str(), _compile(node.constList()[2]).c_str());
                        break;
                    }

                    case Keyword::If:
                    {
                        // (if condition then else)
                        json += fmt::format(
                            R"({{"type": "If", "condition": {}, "then": {}, "else": {}}})",
                            _compile(node.constList()[1]).c_str(), _compile(node.constList()[2]).c_str(), _compile(node.constList()[3]).c_str());
                        break;
                    }

                    case Keyword::While:
                    {
                        // (while condition body)
                        json += fmt::format(
                            R"({{"type": "While", "condition": {}, "body": {}}})",
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
                        // (import pkg.value)
                        // (import pkg.value :sym)
                        // (import pkg.value:*)
                        std::string package = node.constList()[1].constList().front().string();
                        for (const auto& sym : node.constList()[1].constList() | std::views::drop(1))
                            package += "." + sym.string();

                        bool is_glob = node.constList()[2].nodeType() == NodeType::Symbol && node.constList()[2].string() == "*";
                        std::vector<std::string> syms;
                        if (node.constList()[2].nodeType() == NodeType::List)
                        {
                            for (const auto& sym : node.constList()[2].constList())
                                syms.push_back('"' + sym.string() + '"');
                        }
                        json += fmt::format(
                            R"({{"type": "Import", "package": "{}", "glob": {}, "symbols": [{}]}})",
                            package,
                            is_glob,
                            fmt::join(syms, ", "));
                        break;
                    }

                    case Keyword::Del:
                    {
                        // (del value)
                        json += fmt::format(
                            R"({{"type": "Del", "value": {}}})",
                            _compile(node.constList()[1]).c_str());
                        break;
                    }
                }
            }
            else if (node.constList().size() > 1 && node.constList()[0].nodeType() == NodeType::Symbol)
            {
                // (foo bar 1)
                json += fmt::format(
                    R"({{"type": "FunctionCall", "name": {}, "args": )",
                    _compile(node.constList()[0]).c_str());
                json += toJsonList(node, 1) + "}";
            }
            else
                json += toJsonList(node, 0);

            break;
        }

        default:
            throw Ark::Error(fmt::format(
                "Not handled NodeType::{} ({} at {}:{}), please report this error on GitHub",
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
