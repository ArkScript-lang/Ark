#include <Ark/Constants.hpp>
#include <Ark/Compiler/Welder.hpp>

#include <termcolor/proxy.hpp>

#include <Ark/Files.hpp>
#include <Ark/Exceptions.hpp>

namespace Ark
{
    Welder::Welder(const unsigned debug, const std::vector<std::filesystem::path>& lib_env) :
        m_debug(debug), m_importer(debug, lib_env), m_macro_processor(debug), m_optimizer(debug), m_compiler(debug)
    {}

    void Welder::registerSymbol(const std::string& name)
    {
        m_compiler.addDefinedSymbol(name);
    }

    bool Welder::computeASTFromFile(const std::string& filename)
    {
        // get the folder where the script is located
        m_root_file = std::filesystem::path(filename).parent_path();
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
            m_compiler.process(m_optimizer.ast());
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
        if (m_debug >= 1)
            std::cout << "Final bytecode size: " << m_bytecode.size() * sizeof(uint8_t) << "B\n";

        if (m_bytecode.empty())
            return false;

        std::ofstream output(filename, std::ofstream::binary);
        output.write(reinterpret_cast<char*>(&m_bytecode[0]), m_bytecode.size() * sizeof(uint8_t));
        output.close();
        return true;
    }

    const internal::Node& Welder::ast() const noexcept
    {
        return m_optimizer.ast();
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
            m_importer.process(m_root_file, m_parser.ast(), m_parser.imports());
            m_macro_processor.process(m_importer.ast());
            m_optimizer.process(m_macro_processor.ast());

            return true;
        }
        catch (const CodeError& e)
        {
            Diagnostics::generate(e);
            return false;
        }
    }
}
