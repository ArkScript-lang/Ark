/**
 * @file BytecodeReader.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief A bytecode disassembler for ArkScript
 * @version 0.5
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef ARK_COMPILER_BYTECODEREADER_HPP
#define ARK_COMPILER_BYTECODEREADER_HPP

#include <vector>
#include <string>
#include <cinttypes>
#include <optional>

#include <Ark/Platform.hpp>
#include <Ark/Compiler/Common.hpp>
#include <Ark/VM/Value.hpp>

namespace Ark
{
    class State;

    enum class BytecodeSegment
    {
        All,
        Symbols,
        Values,
        Code,
        HeadersOnly
    };

    struct Version
    {
        uint16_t major;
        uint16_t minor;
        uint16_t patch;
    };

    struct Symbols
    {
        std::vector<std::string> symbols {};
        std::size_t start {};  ///< Point to the SYM_TABLE_START byte in the bytecode
        std::size_t end {};    ///< Point to the byte following the last byte of the table in the bytecode
    };

    struct Values
    {
        std::vector<Value> values {};
        std::size_t start {};  ///< Point to the VAL_TABLE_START byte in the bytecode
        std::size_t end {};    ///< Point to the byte following the last byte of the table in the bytecode
    };

    struct Code
    {
        std::vector<bytecode_t> pages {};
        std::size_t start {};  ///< Point to the CODE_SEGMENT_START byte in the bytecode
    };

    /**
     * @brief This class is just a helper to
     * - check if a bytecode is valid
     * - display it in a human readable way by using the opcode names
     *
     */
    class ARK_API BytecodeReader final
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
         * @brief Construct needed data before displaying information about a given bytecode
         *
         * @param bytecode
         */
        void feed(const bytecode_t& bytecode);

        /**
         * Check for the presence of the magic header
         * @return true if the magic 'ark\0' was found
         */
        [[nodiscard]] bool checkMagic() const;

        /**
         * @brief Return the bytecode object constructed
         *
         * @return const bytecode_t&
         */
        [[nodiscard]] const bytecode_t& bytecode() noexcept;

        /**
         *
         * @return Version compiler version used to create the given bytecode file
         */
        [[nodiscard]] Version version() const;

        /**
         * @brief Return the read timestamp from the bytecode file
         *
         * @return unsigned long long
         */
        [[nodiscard]] unsigned long long timestamp() const;

        /**
         *
         * @return std::vector<unsigned char> bytecode sha
         */
        [[nodiscard]] std::vector<unsigned char> sha256() const;

        /**
         *
         * @return Symbols
         */
        [[nodiscard]] Symbols symbols() const;

        /**
         * @param symbols
         * @return Values
         */
        [[nodiscard]] Values values(const Symbols& symbols) const;

        /**
         * @param values
         * @return Code
         */
        [[nodiscard]] Code code(const Values& values) const;

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
                     std::optional<uint16_t> cPage = std::nullopt) const;

        friend class Ark::State;

    private:
        bytecode_t m_bytecode;

        /**
         * @brief Read a number from the bytecode, under the instruction pointer i
         *
         * @param i this parameter is being modified to point to the next value
         * @return uint16_t the number we read (big endian)
         */
        [[nodiscard]] uint16_t readNumber(std::size_t& i) const;
    };
}

#endif
