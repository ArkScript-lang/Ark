#include <Ark/Compiler/ImportSolver.hpp>

#include <ranges>
#include <stack>
#include <fmt/core.h>

#include <Ark/Files.hpp>
#include <Ark/Compiler/AST/Parser.hpp>

namespace Ark::internal
{
    ImportSolver::ImportSolver(const unsigned debug, const std::vector<std::filesystem::path>& libenv) :
        m_debug(debug), m_libenv(libenv), m_ast()
    {}

    void ImportSolver::process(const std::filesystem::path& root, const Node& origin_ast, const std::vector<Import>& origin_imports)
    {
        m_root = root;

        std::stack<Import> imports;
        for (const auto& origin_import : std::ranges::reverse_view(origin_imports))
            imports.push(origin_import);

        while (!imports.empty())
        {
            Import import = imports.top();
            // Remove the top element to process the other imports
            // It needs to be removed first because we might be adding
            // other imports later and don't want to pop THEM
            imports.pop();

            // TODO: add special handling for each type of import (prefixed, with symbols, glob pattern)
            if (!m_modules.contains(import.toPackageString()))
            {
                // NOTE: since the "file" (=root) argument doesn't change between all calls, we could get rid of it
                std::vector<Import> additional_imports = parseImport(root, import);
                // TODO import and store the new node as a Module node.
                //      Module nodes should be scoped relatively to their packages
                //      They should provide specific methods to resolve symbols,
                //      mark them as public or private.
                //      OR we could have a map<import, module>, update the module
                //      accordingly, and once we are done concat all the nodes
                //      in a single AST.
                for (auto& additional_import : std::ranges::reverse_view(additional_imports))
                    imports.push(additional_import);
            }
            else
            {
                // TODO: if we already imported a package we should merge their definition
                //          (import foo:*) > (import foo:a)  -- no prefix
                //          (import foo)  -- with prefix
                //          and then decide what to do with the module
            }
        }

        m_ast = findAndReplaceImports(origin_ast).first;
    }

    std::pair<Node, bool> ImportSolver::findAndReplaceImports(const Node& ast)
    {
        Node x = ast;
        if (x.nodeType() == NodeType::List)
        {
            if (x.constList().size() >= 2 && x.constList()[0].nodeType() == NodeType::Keyword &&
                x.constList()[0].keyword() == Keyword::Import)
            {
                // TODO maybe we'll have problems with :* ?
                std::string package = std::accumulate(
                    std::next(x.constList()[1].constList().begin()),
                    x.constList()[1].constList().end(),
                    x.constList()[1].constList()[0].string(),
                    [](const std::string& acc, const Node& elem) -> std::string {
                        return acc + "." + elem.string();
                    });

                if (std::ranges::find(m_imported, package) == m_imported.end())
                {
                    m_imported.push_back(package);
                    // modules are already handled, we can safely replace the node
                    x = m_modules[package].ast;
                    if (!m_modules[package].has_been_processed)
                        x = findAndReplaceImports(x).first;  // FIXME?
                    return std::make_pair(x, !m_modules[package].has_been_processed);
                }

                // Replace by empty node to avoid breaking the code gen
                x = Node(NodeType::List);
                x.push_back(Node(Keyword::Begin));
            }
            else
            {
                for (std::size_t i = 0; i < x.constList().size(); ++i)
                {
                    auto [node, is_import] = findAndReplaceImports(x.constList()[i]);
                    if (!is_import)
                        x.list()[i] = node;
                    else
                    {
                        if (node.constList().size() > 1)
                        {
                            x.list()[i] = node.constList()[1];
                            // NOTE maybe maybe maybe
                            // why do we start at 2 and not 1?
                            for (std::size_t j = 2, end_j = node.constList().size(); j < end_j; ++j)
                            {
                                if (i + j - 1 < x.list().size())
                                    x.list().insert(x.list().begin() + i + j - 1, node.constList()[j]);
                                else
                                    x.list().push_back(node.constList()[j]);
                            }

                            // -2 because we skipped the Begin node and the first node of the block isn't inserted
                            // but replaces an existing one
                            i += node.constList().size() - 2;
                        }
                        else
                            x.list()[i] = node;
                    }
                }
            }
        }

        return std::make_pair(x, false);
    }

    const Node& ImportSolver::ast() const noexcept
    {
        return m_ast;
    }

    std::vector<Import> ImportSolver::parseImport(const std::filesystem::path& file, const Import& import)
    {
        const auto path = findFile(file, import);
        if (path.extension() == ".arkm")  // Nothing to import in case of modules
        {
            // Creating an import node that will stay there when visiting the AST and
            // replacing the imports with their parsed module
            auto module_node = Node(NodeType::List);
            module_node.push_back(Node(Keyword::Import));

            auto package_node = Node(NodeType::List);
            for (const std::string& stem : import.package)
                package_node.push_back(Node(NodeType::String, stem));
            module_node.push_back(package_node);
            // empty symbols list
            module_node.push_back(Node(NodeType::List));

            m_modules[import.toPackageString()] = Module {
                module_node,
                true
            };

            return {};
        }

        Parser parser;
        const std::string code = Utils::readFile(path);
        parser.process(path.string(), code);
        m_modules[import.toPackageString()] = Module {
            parser.ast(),
            false
        };

        return parser.imports();
    }

    std::optional<std::filesystem::path> testExtensions(const std::filesystem::path& folder, const std::string& package_path)
    {
        if (auto code_path = folder / (package_path + ".ark"); std::filesystem::exists(code_path))
            return code_path;
        if (auto module_path = folder / (package_path + ".arkm"); std::filesystem::exists(module_path))
            return module_path;
        return {};
    }

    std::filesystem::path ImportSolver::findFile(const std::filesystem::path& file, const Import& import)
    {
        const std::string package_path = import.packageToPath();
        if (auto maybe_path = testExtensions(m_root, package_path); maybe_path.has_value())
            return maybe_path.value();

        // search in all folders in environment path
        for (const auto& path : m_libenv)
        {
            if (auto maybe_path = testExtensions(path, package_path); maybe_path.has_value())
                return maybe_path.value();
        }

        // fallback, we couldn't find the file
        throw std::runtime_error(
            fmt::format("While processing file {}, couldn't import {}: file not found",
                        file.generic_string(), import.toPackageString()));
    }
}
