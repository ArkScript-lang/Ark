#include <Ark/Compiler/Package/ImportSolver.hpp>

#include <ranges>
#include <algorithm>
#include <fmt/core.h>

#include <Ark/Utils/Files.hpp>
#include <Ark/Error/Exceptions.hpp>
#include <Ark/Compiler/AST/Parser.hpp>

namespace Ark::internal
{
    ImportSolver::ImportSolver(const unsigned debug, const std::vector<std::filesystem::path>& libenv, Statistics* stats_collector) :
        Pass("ImportSolver", debug, stats_collector),
        m_debug_level(debug),
        m_libenv(libenv)
    {}

    ImportSolver& ImportSolver::setup(const std::filesystem::path& root, const std::vector<Import>& origin_imports)
    {
        // keep the given root if it's a directory, it means it comes from a code string evaluation in the state, where we don't have a filename
        m_root = is_directory(root) ? root : root.parent_path();

        for (const auto& origin_import : std::ranges::reverse_view(origin_imports))
            m_imports.push({ root, "", origin_import });

        return *this;
    }

    void ImportSolver::process(const Node& origin_ast)
    {
        m_logger.traceStart("process");

        while (!m_imports.empty())
        {
            const ImportWithSource source = m_imports.top();
            m_logger.debug("Importing {} from '{}' (in package '{}')", source.import.toPackageString(), source.file.string(), source.package);

            // Remove the top element to process the other imports
            // It needs to be removed first because we might be adding
            // other imports later and don't want to pop THEM
            m_imports.pop();
            const auto package = source.import.toPackageString();

            // register that the current file is importing another one, with its attributes (glob, prefix, symbols...)
            m_package_to_imports[source.package].push_back(source.import);

            if (m_packages.contains(package))
            {
                // todo: actually don't do that, we need to keep a source -> vec<import> that's accurate
                // merge the definition, so that we can generate valid Full Qualified Names in the name & scope resolver
                m_packages[package].import.with_prefix |= source.import.with_prefix;
                m_packages[package].import.is_glob |= source.import.is_glob;
                for (auto&& symbol : source.import.symbols)
                    m_packages[package].import.symbols.push_back(symbol);
            }
            else
            {
                std::vector<ImportWithSource> temp = parseImport(source);
                for (auto& additional_import : std::ranges::reverse_view(temp))
                    m_imports.push(additional_import);
            }
        }

        m_logger.traceStart("discoverAllPackages");
        discoverAllPackages();
        addStat("ImportSolver.discoverAllPackages", m_logger.traceEnd());

        m_logger.traceStart("findAndReplaceImports");
        m_ast = findAndReplaceImports(origin_ast, /* current_namespace_package= */ "").first;
        addStat("ImportSolver.findAndReplaceImports", m_logger.traceEnd());

        addStat("ImportSolver.process", m_logger.traceEnd());
    }

    void ImportSolver::discoverAllPackages()
    {
        for (const std::string& package : std::ranges::views::keys(m_package_to_imports))
        {
            std::vector<Import> import_list = m_package_to_imports[package];
            for (std::size_t i = 0; i < import_list.size(); ++i)
            {
                // if we try to add ourselves to the current import list, we'll end up in an infinite loop
                if (import_list[i].toPackageString() == package)
                    continue;

                // todo: this might fail on unknown imports?
                std::vector<Import> indirect_imports = m_package_to_imports[import_list[i].toPackageString()];
                std::erase_if(indirect_imports, [&import_list](const Import& import) -> bool {
                    return std::ranges::find(import_list, import) != import_list.end();
                });
                for (const Import& import : indirect_imports)
                    import_list.emplace_back(import);
            }

            m_package_to_imports[package] = import_list;
        }
    }

    std::string ImportSolver::importNodeToPackage(const Node& node)
    {
        // compute the package string: foo.bar.egg
        return std::accumulate(
            std::next(node.constList().begin()),
            node.constList().end(),
            node.constList()[0].string(),
            [](const std::string& acc, const Node& elem) -> std::string {
                return acc + "." + elem.string();
            });
    }

    std::pair<Node, bool> ImportSolver::findAndReplaceImports(const Node& ast, const std::string& current_namespace_package)
    {
        Node x = ast;
        if (x.nodeType() == NodeType::List)
        {
            if (x.constList().size() >= 2 && x.constList()[0].nodeType() == NodeType::Keyword &&
                x.constList()[0].keyword() == Keyword::Import)
            {
                const std::string package = importNodeToPackage(x.constList()[1]);

                // if it wasn't imported already, register it
                if (std::ranges::find(m_imported, package) == m_imported.end())
                {
                    statIncrementCount(Stats::ProcessedImports);

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
                            .imports = m_package_to_imports[package],
                            .ast = std::make_shared<Node>(findAndReplaceImports(x, /* current_namespace_package= */ package).first) });

                        x.arkNamespace().ast->setPositionFrom(ast);
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
                    auto [node, is_import] = findAndReplaceImports(x.constList()[i], current_namespace_package);
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

    const std::vector<Import>& ImportSolver::rootImportList()
    {
        // even if we don't have registered any import for the root package, operator[] will create the key "",
        // and we'll be able to return an empty import list, as expected
        return m_package_to_imports[""];
    }

    std::vector<ImportSolver::ImportWithSource> ImportSolver::parseImport(const ImportWithSource& source)
    {
        m_logger.traceStart(fmt::format("parseImport {}", source.file.string()));

        const auto path = findFile(source.file, source.import);
        if (path.extension() == ".arkm")  // Nothing to import in case of modules
        {
            // Creating an import node that will stay there when visiting the AST and
            // replacing the imports with their parsed module
            auto module_node = Node(NodeType::List);
            module_node.push_back(Node(Keyword::Import));

            auto package_node = Node(NodeType::List);
            std::ranges::transform(
                source.import.package,
                std::back_inserter(package_node.list()), [](const std::string& stem) {
                    return Node(NodeType::String, stem);
                });
            module_node.push_back(package_node);
            // empty symbols list
            module_node.push_back(Node(NodeType::List));

            m_packages[source.import.toPackageString()] = Package {
                module_node,
                source.import,
                true
            };

            return {};
        }

        Parser parser(m_debug_level);
        const std::string code = Utils::readFile(path.generic_string());
        parser.process(path.string(), code);
        const std::vector<Import>& imports = parser.imports();

        m_packages[source.import.toPackageString()] = Package {
            parser.ast(),
            source.import,
            false
        };

        addStat(fmt::format("ImportSolver.parseImport({})", source.import.toPackageString()), m_logger.traceEnd());

        std::vector<ImportWithSource> output;
        std::ranges::transform(
            imports,
            std::back_inserter(output), [&path, &source](const Import& i) {
                return ImportWithSource { path, source.import.toPackageString(), i };
            });
        return output;
    }

    std::optional<std::filesystem::path> testExtensions(const std::filesystem::path& folder, const std::string& package_path)
    {
        for (const char* const& ext : { ".ark", ".arkm" })
        {
            auto code_path = folder / (package_path + ext);
            if (std::filesystem::exists(code_path) && std::filesystem::is_regular_file(code_path))
                return code_path;
        }
        return {};
    }

    std::filesystem::path ImportSolver::findFile(const std::filesystem::path& file, const Import& import_) const
    {
        const std::string package_path = import_.packageToPath();
        if (auto maybe_path = testExtensions(m_root, package_path); maybe_path.has_value())
            return maybe_path.value();

        // search in all folders in environment path
        for (const auto& path : m_libenv)
        {
            if (auto maybe_path = testExtensions(path, package_path); maybe_path.has_value())
                return maybe_path.value();
        }

        const bool is_source_file = !is_directory(file);

        // fallback, we couldn't find the file
        throw CodeError(
            fmt::format("While processing file {}, couldn't import {}: file not found",
                        is_source_file ? file.filename().string() : ARK_NO_NAME_FILE, import_.toPackageString()),
            CodeErrorContext(
                is_source_file ? file.generic_string() : ARK_NO_NAME_FILE,
                FileSpan { .start = FilePos { .line = import_.line, .column = import_.col }, .end = std::nullopt }));
    }
}
