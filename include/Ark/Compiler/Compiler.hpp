#ifndef ark_compiler
#define ark_compiler

#include <vector>
#include <iostream>
#include <string>
#include <cinttypes>
#include <optional>

#include <Ark/Parser/Parser.hpp>
#include <Ark/Parser/Node.hpp>
#include <Ark/Compiler/CValue.hpp>
#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/FFI/FFI.hpp>

namespace Ark
{
    class Compiler
    {
    public:
        Compiler(unsigned debug, const std::string& lib_dir, uint16_t options=DefaultFeatures);

        void feed(const std::string& code, const std::string& filename="FILE");
        void compile();
        void saveTo(const std::string& file);

        const bytecode_t& bytecode();

    private:
        Ark::Parser m_parser;
        uint16_t m_options;
        // tables: symbols, values, plugins and codes
        std::vector<std::string> m_symbols;
        std::vector<internal::CValue> m_values;
        std::vector<std::string> m_plugins;
        std::vector<std::vector<internal::Inst>> m_code_pages;
            // we need a temp code pages for some compilations passes
        std::vector<std::vector<internal::Inst>> m_temp_pages;

        bytecode_t m_bytecode;
        unsigned m_debug;

        // helper functions to get a temp or finalized code page
        inline std::vector<internal::Inst>& page(int i);
        // checking if a symbol is an operator or a builtin
        // because they are implemented the same way
        inline std::optional<std::size_t> isOperator(const std::string& name);
        inline std::optional<std::size_t> isBuiltin(const std::string& name);

        // to compile a single node, recursively
        void _compile(Ark::internal::Node x, int p);
        // register a symbol/value/plugin in its own table
        std::size_t addSymbol(const std::string& sym);
        std::size_t addValue(Ark::internal::Node x);
        std::size_t addValue(std::size_t page_id);
        void addPlugin(Ark::internal::Node x);
        // push a number on stack (need 2 )
        void pushNumber(uint16_t n, std::vector<internal::Inst>* page=nullptr);
    };

    #include "Compiler.inl"
}

#endif