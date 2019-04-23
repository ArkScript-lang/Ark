#ifndef ark_compiler
#define ark_compiler

#include <vector>
#include <iostream>
#include <string>
#include <cinttypes>

#include <Ark/Lang/Environment.hpp>
#include <Ark/Parser/Parser.hpp>
#include <Ark/Lang/Node.hpp>
#include <Ark/Compiler/Value.hpp>
#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>

namespace Ark
{
    namespace Compiler
    {
        using namespace Ark::Lang;

        class Compiler
        {
        public:
            Compiler(bool debug=false);
            ~Compiler();

            void feed(const std::string& file);
            void compile();
            void saveTo(const std::string& file);

        private:
            inline std::vector<Inst>& page(int i)
            {
                if (i >= 0)
                    return m_code_pages[i];
                return m_temp_pages[-i - 1];
            }

            void _compile(Node x, int p);
            std::size_t addSymbol(const std::string& sym);
            std::size_t addValue(Node x);
            std::size_t addValue(std::size_t page_id);

            void pushNumber(uint16_t n, std::vector<Inst>* page=nullptr);

            Ark::Parser::Parser m_parser;
            std::vector<std::string> m_symbols;
            std::vector<Value> m_values;
            std::vector<std::vector<Inst>> m_code_pages;
            std::vector<std::vector<Inst>> m_temp_pages;

            bytecode_t m_bytecode;

            bool m_debug;
        };
    }
}

#endif