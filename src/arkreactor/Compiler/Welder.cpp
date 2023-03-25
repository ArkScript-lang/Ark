#include <Ark/Compiler/Welder.hpp>

#include <termcolor/proxy.hpp>

#include <Ark/Files.hpp>
#include <Ark/Exceptions.hpp>

namespace Ark
{
    Welder::Welder(unsigned debug, [[maybe_unused]] const std::vector<std::string>&) :
        m_debug(debug), m_macro_processor(debug), m_optimizer(debug), m_compiler(debug)
    {}

    void Welder::registerSymbol(const std::string& name)
    {
        m_compiler.addDefinedSymbol(name);
    }

    void Welder::computeASTFromFile(const std::string& filename)
    {
        m_root_file = std::filesystem::path(filename);

        try
        {
            m_parser.processFile(m_root_file.string());  // we could have something like m_parser.imports()
            // TODO solve imports
            // needs: root ast+path+imports (vector<T>, T = {withPrefix: boolean, vector<string>: symbolsToImport})
            // 1st draft: ignore symbolsToImport and prefix (current behavior)
            // symbolsToImport -> need to hide all other symbols (prefix with '#') (ast visitor)
            //                 -> use modules/namespaces to do name resolution
            m_macro_processor.process(m_parser.ast());
            m_optimizer.process(m_macro_processor.ast());
        }
        catch (const CodeError& e)
        {
            Diagnostics::generate(e);
        }
    }

    void Welder::computeASTFromString(const std::string& code)
    {
        m_root_file = std::filesystem::current_path();  // No filename given, take the current working directory

        try
        {
            m_parser.processString(code);
            // TODO mutualise this piece of code
            m_macro_processor.process(m_parser.ast());
            m_optimizer.process(m_macro_processor.ast());
        }
        catch (const CodeError& e)
        {
            Diagnostics::generate(e, code);
        }
    }

    void Welder::generateBytecode()
    {
        try
        {
            m_compiler.process(m_optimizer.ast());
            m_bytecode = m_compiler.bytecode();
        }
        catch (const CodeError& e)
        {
            Diagnostics::generate(e);
        }
    }

    void Welder::saveBytecodeToFile(const std::string& filename)
    {
        if (m_debug >= 1)
            std::cout << "Final bytecode size: " << m_bytecode.size() * sizeof(uint8_t) << "B\n";

        std::ofstream output(filename, std::ofstream::binary);
        output.write(reinterpret_cast<char*>(&m_bytecode[0]), m_bytecode.size() * sizeof(uint8_t));
        output.close();
    }

    const internal::Node& Welder::ast() const noexcept
    {
        return m_optimizer.ast();
    }

    const bytecode_t& Welder::bytecode() const noexcept
    {
        return m_bytecode;
    }
}
