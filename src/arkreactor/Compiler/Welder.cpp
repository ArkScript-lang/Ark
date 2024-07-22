#include <Ark/Constants.hpp>
#include <Ark/Compiler/Welder.hpp>

#include <Ark/Compiler/ImportSolver.hpp>
#include <Ark/Compiler/AST/Optimizer.hpp>
#include <Ark/Compiler/Macros/Processor.hpp>

#include <Ark/Files.hpp>
#include <Ark/Exceptions.hpp>

namespace Ark
{
    Welder::Welder(const unsigned debug, const std::vector<std::filesystem::path>& lib_env, const uint16_t features) :
        Pass("Welder", debug), m_lib_env(lib_env), m_features(features),
        m_computed_ast(internal::NodeType::Unused), m_compiler(debug)
    {}

    void Welder::registerSymbol(const std::string& name)
    {
        m_compiler.addDefinedSymbol(name);
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
            m_bytecode = m_compiler.bytecode();

            return true;
        }
        catch (const CodeError& e)
        {
            Diagnostics::generate(e);
            return false;
        }
    }

    bool Welder::saveBytecodeToFile(const std::string& filename)
    {
        log("Final bytecode size: {}B", m_bytecode.size() * sizeof(uint8_t));

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

    bool Welder::computeAST(const std::string& filename, const std::string& code)
    {
        try
        {
            m_parser.process(filename, code);

            if ((m_features & FeatureImportSolver) != 0)
            {
                auto import_solver_pass = std::make_unique<internal::ImportSolver>(debugLevel(), m_lib_env);
                import_solver_pass->setup(m_root_file, m_parser.imports());
                m_passes.push_back(std::move(import_solver_pass));
            }
            if ((m_features & FeatureMacroProcessor) != 0)
                m_passes.emplace_back(std::make_unique<internal::MacroProcessor>(debugLevel()));
            if ((m_features & FeatureASTOptimizer) != 0)
                m_passes.emplace_back(std::make_unique<internal::Optimizer>(debugLevel()));

            m_computed_ast = std::accumulate(
                m_passes.begin(),
                m_passes.end(),
                m_parser.ast(),
                [](const internal::Node& ast, const std::unique_ptr<internal::Pass>& pass) {
                    pass->process(ast);
                    return pass->ast();
                });

            return true;
        }
        catch (const CodeError& e)
        {
            Diagnostics::generate(e);
            return false;
        }
    }
}
