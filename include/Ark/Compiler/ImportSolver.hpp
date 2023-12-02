#ifndef ARK_COMPILER_IMPORTSOLVER_HPP
#define ARK_COMPILER_IMPORTSOLVER_HPP

#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>

#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/AST/Import.hpp>
#include <Ark/Compiler/AST/Module.hpp>

namespace Ark::internal
{
    class ImportSolver final
    {
    public:
        ImportSolver(unsigned debug, const std::vector<std::filesystem::path>& libenv);

        void process(const std::filesystem::path& root, const Node& origin_ast, const std::vector<Import>& origin_imports);

        [[nodiscard]] const Node& ast() const noexcept;

    private:
        unsigned m_debug;
        std::vector<std::filesystem::path> m_libenv;
        std::filesystem::path m_root;  ///< Folder were the entry file is
        Node m_ast;
        std::unordered_map<std::string, Module> m_modules;  ///< Package to module map
        // TODO is this ok? is this fine? this is sort of ugly
        std::vector<std::string> m_imported;  ///< List of imports, in the order they were found and parsed

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
         * @param file path to the file containing the import
         * @param import current import directive
         * @return std::vector<Import> imports found in the processed file
         */
        std::vector<Import> parseImport(const std::filesystem::path& file, const Import& import);

        /**
         * @brief Search for an import file, using the root file path
         * 
         * @param file path to the file containing the import
         * @param import current import directive
         * @return std::filesystem::path 
         */
        std::filesystem::path findFile(const std::filesystem::path& file, const Import& import);
    };
}

#endif
