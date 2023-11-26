/**
 * @file Welder.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief In charge of welding everything needed to compile code
 * @version 0.2
 * @date 2023-03-26
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef ARK_COMPILER_WELDER_HPP
#define ARK_COMPILER_WELDER_HPP

#include <string>
#include <memory>
#include <vector>
#include <filesystem>

#include <Ark/Compiler/Common.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/AST/Parser.hpp>
#include <Ark/Compiler/ImportSolver.hpp>
#include <Ark/Compiler/AST/Optimizer.hpp>
#include <Ark/Compiler/Macros/Processor.hpp>
#include <Ark/Compiler/Compiler.hpp>

namespace Ark
{
    class ARK_API Welder final
    {
    public:
        Welder(unsigned debug, const std::vector<std::filesystem::path>& libenv);

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

        const internal::Node& ast() const noexcept;
        const bytecode_t& bytecode() const noexcept;

    private:
        unsigned m_debug;  ///< The debug level
        std::filesystem::path m_root_file;
        std::vector<std::string> m_imports;
        bytecode_t m_bytecode;

        internal::Parser m_parser;
        internal::ImportSolver m_importer;
        internal::MacroProcessor m_macro_processor;
        internal::Optimizer m_optimizer;
        Compiler m_compiler;
    };
}  // namespace Ark

#endif
