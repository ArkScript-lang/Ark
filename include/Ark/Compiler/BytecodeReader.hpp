/**
 * @file BytecodeReader.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief A bytecode disassembler for ArkScript
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef ark_compiler_bytecodereader
#define ark_compiler_bytecodereader

#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <cinttypes>

#include <Ark/Config.hpp>

namespace Ark
{
    using bytecode_t = std::vector<uint8_t>;

    /**
     * @brief This class is just a helper to
     * - check if a bytecode is valid
     * - display it in a human readable way by using the opcode names
     * 
     */
    class ARK_API_EXPORT BytecodeReader
    {
    public:
        /**
         * @brief Construct a new Bytecode Reader object
         * 
         */
        BytecodeReader() = default;

        /**
         * @brief Construct needed data before displaying information about a given file
         * 
         * @param file filename of the bytecode file
         */
        void feed(const std::string& file);

        /**
         * @brief Return the bytecode object constructed
         * 
         * @return const bytecode_t& 
         */
        const bytecode_t& bytecode() noexcept;

        /**
         * @brief Return the read timestamp from the bytecode file
         * 
         * @return unsigned long long 
         */
        unsigned long long timestamp();

        /**
         * @brief Display the bytecode opcode in a human friendly way
         * 
         */
        void display();

    private:
        bytecode_t m_bytecode;

        uint16_t readNumber(std::size_t& i);
    };
}

#endif