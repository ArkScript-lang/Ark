/**
 * @file ImportSolver.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Handle imports, resolve them with modules and everything
 * @date 2024-07-21
 *
 * @copyright Copyright (c) 2020-2026
 *
 */

#ifndef ARK_COMPILER_IMPORTSOLVER_HPP
#define ARK_COMPILER_IMPORTSOLVER_HPP

#include <stack>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>

#include <Ark/Utils/Platform.hpp>
#include <Ark/Compiler/Pass.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/AST/Import.hpp>
#include <Ark/Compiler/Package/Package.hpp>

namespace Ark::internal
{
    class ARK_API ImportSolver final : public Pass
    {
    public:
        /**
         * @brief Create a new ImportSolver
         * @param debug debug level
         * @param libenv list of paths to the standard library
         */
        ImportSolver(unsigned debug, const std::vector<std::filesystem::path>& libenv);

        /**
         * @brief Configure the ImportSolver
         * @param root path to the root file that imports all others
         * @param origin_imports the first imports to go through
         * @return ImportSolver& *this
         */
        ImportSolver& setup(const std::filesystem::path& root, const std::vector<Import>& origin_imports);

        void process(const Node& origin_ast) override;

        [[nodiscard]] const Node& ast() const noexcept override;

    private:
        struct ImportWithSource
        {
            std::filesystem::path file;
            Import import;
        };

        unsigned m_debug_level;
        std::vector<std::filesystem::path> m_libenv;
        std::filesystem::path m_root;  ///< Folder were the entry file is
        Node m_ast;
        std::stack<ImportWithSource> m_imports;
        std::unordered_map<std::string, Package> m_packages;  ///< Package name to package AST & data mapping
        std::vector<std::string> m_imported;                  ///< List of imports, in the order they were found and parsed

        /**
         * @brief Visits the AST, looking for import nodes to replace with their parsed module version
         * @param ast
         * @return
         */
        std::pair<Node, bool> findAndReplaceImports(const Node& ast);

        /**
         * @brief Parse a given file and returns a list of its imports.
         *        The AST is parsed and stored in m_modules[import.prefix]
         *
         * @param source path to the file containing the import
         * @param import current import directive
         * @return std::vector<ImportWithSource> imports found in the processed file
         */
        std::vector<ImportWithSource> parseImport(const std::filesystem::path& source, const Import& import);

        /**
         * @brief Search for an import file, using the root file path
         *
         * @param file path to the file containing the import
         * @param import_ current import directive
         * @return std::filesystem::path
         */
        [[nodiscard]] std::filesystem::path findFile(const std::filesystem::path& file, const Import& import_) const;
    };
}

#endif
