#ifndef ark_compiler
#define ark_compiler

#include <vector>
#include <iostream>
#include <string>
#include <cinttypes>
#include <optional>

#include <Ark/Parser/Parser.hpp>
#include <Ark/Parser/Node.hpp>
#include <Ark/Compiler/Value.hpp>
#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/VM/FFI.hpp>

namespace Ark
{
    class Compiler
    {
    public:
        Compiler(bool debug, const std::string& lib_dir, uint16_t options);

        void feed(const std::string& code, const std::string& filename="FILE");
        void compile();
        void saveTo(const std::string& file);

        const bytecode_t& bytecode();

    private:
        Ark::Parser m_parser;
        uint16_t m_options;
        std::vector<std::string> m_symbols;
        std::vector<internal::CValue> m_values;
        std::vector<std::string> m_plugins;
        std::vector<std::vector<internal::Inst>> m_code_pages;
        std::vector<std::vector<internal::Inst>> m_temp_pages;

        bytecode_t m_bytecode;

        bool m_debug;

        inline std::vector<internal::Inst>& page(int i)
        {
            if (i >= 0)
                return m_code_pages[i];
            return m_temp_pages[-i - 1];
        }

        inline std::optional<std::size_t> isOperator(const std::string& name)
        {
            auto it = std::find(internal::FFI::operators.begin(), internal::FFI::operators.end(), name);
            if (it != internal::FFI::operators.end())
                return std::distance(internal::FFI::operators.begin(), it);
            return {};
        }

        inline std::optional<std::size_t> isBuiltin(const std::string& name)
        {
            auto it = std::find_if(internal::FFI::builtins.begin(), internal::FFI::builtins.end(),
            [&name](const std::pair<std::string, internal::Value>& element) -> bool {
                return name == element.first;
            });
            if (it != internal::FFI::builtins.end())
                return std::distance(internal::FFI::builtins.begin(), it);
            return {};
        }

        void _compile(Ark::internal::Node x, int p);
        std::size_t addSymbol(const std::string& sym);
        std::size_t addValue(Ark::internal::Node x);
        std::size_t addValue(std::size_t page_id);
        void addPlugin(Ark::internal::Node x);

        void pushNumber(uint16_t n, std::vector<internal::Inst>* page=nullptr);
    };
}

#endif