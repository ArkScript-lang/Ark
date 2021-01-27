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
#include <optional>

#include <Ark/Config.hpp>

namespace Ark
{
    using bytecode_t = std::vector<uint8_t>;

    enum class BytecodeSegment {
        All,
        Symbols,
        Values,
        Code
    };
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
         * @brief Display the bytecode opcode in a human friendly way.
         *
         * @param segment selected bytecode segment that will be displayed
         * @param sStart start of the segment slice to display (Ignored in code segment if no page is available)
         * @param sEnd end of the segment slice to display (Ignored in code segment if no page is available)
         * @param cPage selected page of the code segment (Used only for the code segment)
         */
        void display(BytecodeSegment segment = BytecodeSegment::All,
                     std::optional<uint16_t> sStart = std::nullopt,
                     std::optional<uint16_t> sEnd = std::nullopt,
                     std::optional<uint16_t> cPage = std::nullopt);

    private:
        bytecode_t m_bytecode;

        uint16_t readNumber(std::size_t& i);
    };
}

#endif