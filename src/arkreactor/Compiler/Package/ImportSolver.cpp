#include <Ark/Compiler/Package/ImportSolver.hpp>

#include <ranges>
#include <algorithm>
#include <fmt/core.h>

#include <Ark/Utils/Files.hpp>
#include <Ark/Error/Exceptions.hpp>
#include <Ark/Compiler/AST/Parser.hpp>

namespace Ark::internal
{
    ImportSolver::ImportSolver(const unsigned debug, const std::vector<std::filesystem::path>& libenv) :
        Pass("ImportSolver", debug), m_debug_level(debug), m_libenv(libenv), m_ast()
    {}

    ImportSolver& ImportSolver::setup(const std::filesystem::path& root, const std::vector<Import>& origin_imports)
    {
        m_root = root.parent_path();

        for (const auto& origin_import : std::ranges::reverse_view(origin_imports))
            m_imports.push({ root, origin_import });

        return *this;
    }

    void ImportSolver::process(const Node& origin_ast)
    {
        m_logger.traceStart("process");

        while (!m_imports.empty())
        {
            ImportWithSource source = m_imports.top();
            m_logger.debug("Importing {}", source.import.toPackageString());

            // Remove the top element to process the other imports
            // It needs to be removed first because we might be adding
            // other imports later and don't want to pop THEM
            m_imports.pop();
            const auto package = source.import.toPackageString();

            if (m_packages.contains(package))
            {
                // merge the definition, so that we can generate valid Full Qualified Names in the name & scope resolver
                m_packages[package].import.with_prefix |= source.import.with_prefix;
                m_packages[package].import.is_glob |= source.import.is_glob;
                for (auto&& symbol : source.import.symbols)
                    m_packages[package].import.symbols.push_back(symbol);
            }
            else
            {
                // NOTE: since the "file" (=root) argument doesn't change between all calls, we could get rid of it
                std::vector<ImportWithSource> temp = parseImport(source.file, source.import);
                for (auto& additional_import : std::ranges::reverse_view(temp))
                    m_imports.push(additional_import);
            }
        }

        m_logger.traceStart("findAndReplaceImports");
        m_ast = findAndReplaceImports(origin_ast).first;
        m_logger.traceEnd();

        m_logger.traceEnd();
    }

    std::pair<Node, bool> ImportSolver::findAndReplaceImports(const Node& ast)
    {
        Node x = ast;
        if (x.nodeType() == NodeType::List)
        {
            if (x.constList().size() >= 2 && x.constList()[0].nodeType() == NodeType::Keyword &&
                x.constList()[0].keyword() == Keyword::Import)
            {
                // compute the package string: foo.bar.egg
                const auto import_node = x.constList()[1].constList();
                const std::string package = std::accumulate(
                    std::next(import_node.begin()),
                    import_node.end(),
                    import_node[0].string(),
                    [](const std::string& acc, const Node& elem) -> std::string {
                        return acc + "." + elem.string();
                    });

                // if it wasn't imported already, register it
                if (std::ranges::find(m_imported, package) == m_imported.end())
                {
                    m_imported.push_back(package);
                    // modules are already handled, we can safely replace the node
                    x = m_packages[package].ast;
                    if (!m_packages[package].has_been_processed)
                    {
                        const auto import = m_packages[package].import;

                        // prefix to lowercase ; usually considered unsafe (https://devblogs.microsoft.com/oldnewthing/20241007-00/?p=110345)
                        // but we are dealing with prefix from filenames, thus we can somewhat assume we are in safe zone
                        std::string prefix = import.prefix;
                        std::ranges::transform(
                            prefix, prefix.begin(),
                            [](auto c) {
                                return std::tolower(c);
                            });

                        x = Node(Namespace {
                            .name = prefix,
                            .is_glob = import.is_glob,
                            .with_prefix = import.with_prefix,
                            .symbols = import.symbols,
                            .ast = std::make_shared<Node>(findAndReplaceImports(x).first) });
                        x.arkNamespace().ast->setPos(ast.line(), ast.col());
                        x.arkNamespace().ast->setFilename(ast.filename());
                    }
                    // we parsed an import node, return true in the pair to notify the caller
                    return std::make_pair(x, /* is_import= */ true);
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
                    x.list()[i] = node;
                }
            }
        }

        return std::make_pair(x, /* is_import= */ false);
    }

    const Node& ImportSolver::ast() const noexcept
    {
        return m_ast;
    }

    std::vector<ImportSolver::ImportWithSource> ImportSolver::parseImport(const std::filesystem::path& source, const Import& import)
    {
        m_logger.traceStart(fmt::format("parseImport {}", source.string()));

        const auto path = findFile(source, import);
        if (path.extension() == ".arkm")  // Nothing to import in case of modules
        {
            // Creating an import node that will stay there when visiting the AST and
            // replacing the imports with their parsed module
            auto module_node = Node(NodeType::List);
            module_node.push_back(Node(Keyword::Import));

            auto package_node = Node(NodeType::List);
            std::ranges::transform(
                import.package,
                std::back_inserter(package_node.list()), [](const std::string& stem) {
                    return Node(NodeType::String, stem);
                });
            module_node.push_back(package_node);
            // empty symbols list
            module_node.push_back(Node(NodeType::List));

            m_packages[import.toPackageString()] = Package {
                module_node,
                import,
                true
            };

            return {};
        }

        Parser parser(m_debug_level);
        const std::string code = Utils::readFile(path.generic_string());
        parser.process(path.string(), code);
        m_packages[import.toPackageString()] = Package {
            parser.ast(),
            import,
            false
        };

        m_logger.traceEnd();

        auto imports = parser.imports();
        std::vector<ImportWithSource> output;
        std::ranges::transform(
            imports,
            std::back_inserter(output), [&path](const Import& i) {
                return ImportWithSource { path, i };
            });
        return output;
    }

    std::optional<std::filesystem::path> testExtensions(const std::filesystem::path& folder, const std::string& package_path)
    {
        if (auto code_path = folder / (package_path + ".ark"); std::filesystem::exists(code_path))
            return code_path;
        if (auto module_path = folder / (package_path + ".arkm"); std::filesystem::exists(module_path))
            return module_path;
        return {};
    }

    std::filesystem::path ImportSolver::findFile(const std::filesystem::path& file, const Import& import) const
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
        throw CodeError(
            fmt::format("While processing file {}, couldn't import {}: file not found",
                        file.filename().string(), import.toPackageString()),
            CodeErrorContext(
                file.generic_string(),
                import.line,
                import.col,
                fmt::format("(import {})", import.toPackageString())));
    }
}
