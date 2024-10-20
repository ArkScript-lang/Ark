#include <Ark/Constants.hpp>
#include <Ark/Compiler/Welder.hpp>

#include <Ark/Compiler/Package/ImportSolver.hpp>
#include <Ark/Compiler/AST/Optimizer.hpp>
#include <Ark/Compiler/Macros/Processor.hpp>
#include <Ark/Compiler/NameResolutionPass.hpp>
#include <Ark/Files.hpp>
#include <Ark/Exceptions.hpp>

#include <fmt/ostream.h>

namespace Ark
{
    Welder::Welder(const unsigned debug, const std::vector<std::filesystem::path>& lib_env, const uint16_t features) :
        m_lib_env(lib_env), m_features(features),
        m_computed_ast(internal::NodeType::Unused),
        m_parser(debug),
        m_import_solver(debug, lib_env),
        m_macro_processor(debug),
        m_ast_optimizer(debug),
        m_name_resolver(debug),
        m_logger("Welder", debug),
        m_ir_optimizer(debug),
        m_ir_compiler(debug),
        m_compiler(debug)
    {}

    void Welder::registerSymbol(const std::string& name)
    {
        m_name_resolver.addDefinedSymbol(name, /* is_mutable= */ false);
    }

    bool Welder::computeASTFromFile(const std::string& filename)
    {
        m_root_file = std::filesystem::path(filename);
        const std::string code = Utils::readFile(filename);

        return computeAST(filename, code);
    }

    bool Welder::computeASTFromString(const std::string& code)
    {
        m_root_file = std::filesystem::current_path();  // No filename given, take the current working directory

        return computeAST(ARK_NO_NAME_FILE, code);
    }

    bool Welder::generateBytecode()
    {
        try
        {
            m_compiler.process(m_computed_ast);
            m_ir = m_compiler.intermediateRepresentation();

            if ((m_features & FeatureIROptimizer) != 0)
            {
                m_ir_optimizer.process(m_ir, m_compiler.symbols(), m_compiler.values());
                m_ir = m_ir_optimizer.intermediateRepresentation();
            }

            if ((m_features & FeatureDumpIR) != 0)
                dumpIRToFile();

            m_ir_compiler.process(m_ir, m_compiler.symbols(), m_compiler.values());
            m_bytecode = m_ir_compiler.bytecode();

            return true;
        }
        catch (const CodeError& e)
        {
            if ((m_features & FeatureTestFailOnException) > 0)
                throw;

            Diagnostics::generate(e);
            return false;
        }
    }

    bool Welder::saveBytecodeToFile(const std::string& filename)
    {
        m_logger.info("Final bytecode size: {}B", m_bytecode.size() * sizeof(uint8_t));

        if (m_bytecode.empty())
            return false;

        std::ofstream output(filename, std::ofstream::binary);
        output.write(
            reinterpret_cast<char*>(&m_bytecode[0]),
            static_cast<std::streamsize>(m_bytecode.size() * sizeof(uint8_t)));
        output.close();
        return true;
    }

    const internal::Node& Welder::ast() const noexcept
    {
        return m_computed_ast;
    }

    const bytecode_t& Welder::bytecode() const noexcept
    {
        return m_bytecode;
    }

    void Welder::dumpIRToFile() const
    {
        std::filesystem::path path = m_root_file;
        if (is_directory(m_root_file))
            path /= "output.ark.ir";
        else
            path.replace_extension(".ark.ir");

        std::ofstream output(path);

        std::size_t index = 0;
        for (const auto& block : m_ir)
        {
            fmt::println(output, "page_{}", index);
            for (const auto entity : block)
            {
                switch (entity.kind())
                {
                    case internal::IR::Kind::Label:
                        fmt::println(output, ".L{}:", entity.label());
                        break;

                    case internal::IR::Kind::Goto:
                        fmt::println(output, "\tGOTO L{}", entity.label());
                        break;

                    case internal::IR::Kind::GotoIfTrue:
                        fmt::println(output, "\tGOTO_IF_TRUE L{}", entity.label());
                        break;

                    case internal::IR::Kind::GotoIfFalse:
                        fmt::println(output, "\tGOTO_IF_FALSE L{}", entity.label());
                        break;

                    case internal::IR::Kind::Opcode:
                        fmt::println(output, "\t{} {}", internal::InstructionNames[entity.inst()], entity.primaryArg());
                        break;

                    case internal::IR::Kind::Opcode2Args:
                        fmt::println(output, "\t{} {}, {}", internal::InstructionNames[entity.inst()], entity.primaryArg(), entity.secondaryArg());
                        break;
                }
            }

            fmt::println(output, "");
            ++index;
        }

        output.close();
    }

    bool Welder::computeAST(const std::string& filename, const std::string& code)
    {
        try
        {
            m_parser.process(filename, code);
            m_computed_ast = m_parser.ast();

            if ((m_features & FeatureImportSolver) != 0)
            {
                m_import_solver.setup(m_root_file, m_parser.imports());
                m_import_solver.process(m_computed_ast);
                m_computed_ast = m_import_solver.ast();
            }

            if ((m_features & FeatureMacroProcessor) != 0)
            {
                m_macro_processor.process(m_computed_ast);
                m_computed_ast = m_macro_processor.ast();
            }

            if ((m_features & FeatureASTOptimizer) != 0)
            {
                m_ast_optimizer.process(m_computed_ast);
                m_computed_ast = m_ast_optimizer.ast();
            }

            if ((m_features & FeatureNameResolver) != 0)
            {
                // NOTE: ast isn't modified by the name resolver, no need to update m_computed_ast
                m_name_resolver.process(m_computed_ast);
            }

            return true;
        }
        catch (const CodeError& e)
        {
            if ((m_features & FeatureTestFailOnException) > 0)
                throw;

            Diagnostics::generate(e);
            return false;
        }
    }
}
