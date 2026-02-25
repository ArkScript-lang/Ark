#include <Ark/Constants.hpp>
#include <Ark/Compiler/Welder.hpp>

#include <Ark/Compiler/Package/ImportSolver.hpp>
#include <Ark/Compiler/AST/Optimizer.hpp>
#include <Ark/Compiler/Macros/Processor.hpp>
#include <Ark/Compiler/NameResolution/NameResolutionPass.hpp>
#include <Ark/Utils/Files.hpp>
#include <Ark/Error/Exceptions.hpp>
#include <Ark/Error/Diagnostics.hpp>
#include <Ark/VM/Value/Value.hpp>

#include <cassert>
#include <sstream>
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
        m_lowerer(debug),
        m_ir_optimizer(debug),
        m_ir_compiler(debug)
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

    bool Welder::computeASTFromStringWithKnownSymbols(const std::string& code, const std::vector<std::string>& symbols)
    {
        m_root_file = std::filesystem::current_path();  // No filename given, take the current working directory

        for (const std::string& sym : symbols)
            m_name_resolver.addDefinedSymbol(sym, /* is_mutable= */ true);
        return computeAST(ARK_NO_NAME_FILE, code);
    }

    bool Welder::generateBytecode()
    {
        try
        {
            m_lowerer.process(m_computed_ast);
            m_ir = m_lowerer.intermediateRepresentation();

            if ((m_features & FeatureIROptimizer) != 0)
            {
                m_ir_optimizer.process(m_ir, m_lowerer.symbols(), m_lowerer.values());
                m_ir = m_ir_optimizer.intermediateRepresentation();
            }

            m_ir_compiler.process(m_ir, m_lowerer.symbols(), m_lowerer.values());
            m_bytecode = m_ir_compiler.bytecode();

            if ((m_features & FeatureDumpIR) != 0)
                dumpIRToFile();

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

    bool Welder::generateBytecodeUsingTables(const std::vector<std::string>& symbols, const std::vector<Value>& constants, const std::size_t start_page_at_offset)
    {
        std::vector<internal::ValTableElem> values;
        for (const Value& constant : constants)
        {
            switch (constant.valueType())
            {
                case ValueType::Number:
                    values.emplace_back(constant.number());
                    break;

                case ValueType::String:
                    values.emplace_back(constant.string());
                    break;

                case ValueType::PageAddr:
                    values.emplace_back(static_cast<std::size_t>(constant.pageAddr()));
                    break;

                default:
                    assert(false && "This should not be possible to have a constant that isn't a Number, a String or a PageAddr");
                    break;
            }
        }

        m_lowerer.addToTables(symbols, values);
        m_lowerer.offsetPagesBy(start_page_at_offset);
        return generateBytecode();
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

    std::string Welder::textualIR() const noexcept
    {
        std::stringstream stream;
        m_ir_compiler.dumpToStream(stream);
        return stream.str();
    }

    const bytecode_t& Welder::bytecode() const noexcept
    {
        return m_bytecode;
    }

    void Welder::dumpIRToFile() const
    {
        std::filesystem::path path = m_root_file;
        if (is_directory(m_root_file))
            path = path / ARK_CACHE_DIRNAME / "output.ark.ir";
        else
        {
            const auto filename = path.filename().replace_extension(".ark.ir");
            path.remove_filename();
            path = path / ARK_CACHE_DIRNAME / filename;
        }

        std::ofstream output(path);
        m_ir_compiler.dumpToStream(output);
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

            if ((m_features & FeatureNameResolver) != 0)
            {
                m_name_resolver.process(m_computed_ast);
                m_computed_ast = m_name_resolver.ast();
            }

            if ((m_features & FeatureASTOptimizer) != 0)
            {
                m_ast_optimizer.process(m_computed_ast);
                m_computed_ast = m_ast_optimizer.ast();
            }

            return true;
        }
        catch (const CodeError& e)
        {
            if ((m_features & FeatureTestFailOnException) > 0)
                throw;

            if (filename != ARK_NO_NAME_FILE)
                Diagnostics::generate(e);
            else
                Diagnostics::generateWithCode(e, code);
            return false;
        }
    }
}
