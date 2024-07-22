/**
 * @file Welder.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief In charge of welding everything needed to compile code
 * @version 0.4
 * @date 2023-03-26
 *
 * @copyright Copyright (c) 2023-2024
 *
 */

#ifndef ARK_COMPILER_WELDER_HPP
#define ARK_COMPILER_WELDER_HPP

#include <string>
#include <vector>
#include <filesystem>

#include <Ark/Compiler/Common.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/AST/Parser.hpp>
#include <Ark/Compiler/Compiler.hpp>
#include <Ark/Compiler/Pass.hpp>
#include <Ark/Constants.hpp>

namespace Ark
{
    /**
     * @brief The welder joins all the compiler passes, being itself one since it can output an AST too
     */
    class ARK_API Welder final : public internal::Pass
    {
    public:
        Welder(unsigned debug, const std::vector<std::filesystem::path>& lib_env, uint16_t features = DefaultFeatures);

        /**
         * @brief Register a symbol as a global in the compiler
         *
         * @param name
         */
        void registerSymbol(const std::string& name);

        bool computeASTFromFile(const std::string& filename);
        bool computeASTFromString(const std::string& code);

        bool generateBytecode();
        bool saveBytecodeToFile(const std::string& filename);

        [[nodiscard]] const internal::Node& ast() const noexcept override;
        [[nodiscard]] const bytecode_t& bytecode() const noexcept;

    private:
        std::vector<std::filesystem::path> m_lib_env;
        uint16_t m_features;

        std::filesystem::path m_root_file;
        std::vector<std::string> m_imports;
        bytecode_t m_bytecode;
        internal::Node m_computed_ast;

        std::vector<std::unique_ptr<internal::Pass>> m_passes;
        internal::Parser m_parser;
        Compiler m_compiler;

        bool computeAST(const std::string& filename, const std::string& code);

        // HACK so that the parser can be a pass and use the loggers
        void process([[maybe_unused]] const internal::Node&) override {}
    };
}  // namespace Ark

#endif
