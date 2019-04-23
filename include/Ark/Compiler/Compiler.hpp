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

namespace Ark
{
    namespace Compiler
    {
        using namespace Ark::Lang;

        class Compiler
        {
        public:
            Compiler();
            ~Compiler();

            void feed(const std::string& file);
            void compile();

        private:
            void _compile(Node x, std::vector<Inst>& page);
            std::size_t addSymbol(const std::string& sym);
            std::size_t addValue(Node x);
            std::size_t addValue(std::size_t page_id);

            void pushNumber(uint16_t n, std::vector<Inst>* page=nullptr);

            Ark::Parser::Parser m_parser;
            std::vector<std::string> m_symbols;
            std::vector<Value> m_values;
            std::vector<std::vector<Inst>> m_code_pages;
            std::vector<Inst> m_temp_page;

            std::vector<uint8_t> m_bytecode;
        };
    }
}

#endif