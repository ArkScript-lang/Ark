/**
 * @file Entity.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief An entity in the IR is a bundle of information
 * @date 2024-10-05
 *
 * @copyright Copyright (c) 2024-2026
 *
 */

#ifndef ARK_COMPILER_INTERMEDIATEREPRESENTATION_ENTITY_HPP
#define ARK_COMPILER_INTERMEDIATEREPRESENTATION_ENTITY_HPP

#include <cinttypes>
#include <vector>
#include <string>
#include <optional>

#include <Ark/Compiler/IntermediateRepresentation/Word.hpp>
#include <Ark/Compiler/Instructions.hpp>

namespace Ark::internal::IR
{
    enum class Kind
    {
        Label,
        Goto,
        GotoWithArg,
        Opcode,
        Opcode2Args,
        Opcode3Args
    };

    using label_t = std::size_t;

    /// The maximum value an argument can have when an IR entity has two arguments
    constexpr uint16_t MaxValueForDualArg = 0x0fff;
    constexpr uint16_t MaxValueForSmallNumber = 0x0800;
    static_assert(MaxValueForSmallNumber + MaxValueForSmallNumber - 1 == MaxValueForDualArg);

    constexpr std::string AnonymousBlockName = "#anonymous";

    class Entity
    {
    public:
        explicit Entity(Kind kind);

        explicit Entity(Instruction inst, uint16_t arg = 0);

        Entity(Instruction inst, uint16_t primary_arg, uint16_t secondary_arg);

        Entity(Instruction inst, uint8_t inst2, uint8_t inst3, uint8_t inst4);

        void replaceInstruction(Instruction replacement);

        static Entity Label(label_t value);

        static Entity Goto(const Entity& label, Instruction inst = Instruction::JUMP);

        static Entity GotoWithArg(const Entity& label, Instruction inst, uint16_t primary_arg);

        static Entity GotoIf(const Entity& label, bool cond);

        [[nodiscard]] Word bytecode() const;

        [[nodiscard]] label_t label() const { return m_label; }

        [[nodiscard]] Kind kind() const { return m_kind; }

        [[nodiscard]] Instruction inst() const { return m_inst; }

        [[nodiscard]] uint16_t primaryArg() const { return m_primary_arg; }

        [[nodiscard]] uint16_t secondaryArg() const { return m_secondary_arg; }

        [[nodiscard]] uint16_t tertiaryArg() const { return m_tertiary_arg; }

        void setSourceLocation(const std::string& filename, std::size_t line);

        void setOriginalSymbolId(std::optional<uint16_t> id);

        [[nodiscard]] bool hasValidSourceLocation() const { return !m_metadata.source_file.empty(); }

        [[nodiscard]] const std::string& filename() const { return m_metadata.source_file; }

        [[nodiscard]] std::size_t sourceLine() const { return m_metadata.source_line; }

        [[nodiscard]] std::optional<uint16_t> originalSymbolId() const { return m_metadata.symbol_id; }

    private:
        Kind m_kind;
        label_t m_label { 0 };

        Instruction m_inst { NOP };
        uint16_t m_primary_arg { 0 };
        uint16_t m_secondary_arg { 0 };
        uint16_t m_tertiary_arg { 0 };

        struct
        {
            std::string source_file;
            std::size_t source_line { 0 };
            std::optional<uint16_t> symbol_id;  ///< Used for LOAD_SYMBOL_BY_INDEX to know the original symbol id and deoptimize when necessary
        } m_metadata;
    };

    struct Block
    {
        using vec_t = std::vector<Entity>;

        struct Metadata
        {
            std::optional<std::string> name;
            std::size_t argument_count { 0 };
            bool is_closure { false };
            bool is_recursive { false };
            bool is_simple { false };  ///< Calls only builtin and operators, no user functions/C++ functions
        } metadata;
        vec_t data;

        [[nodiscard]] std::string debugName() const
        {
            return metadata.name.value_or(AnonymousBlockName);
        }

        [[nodiscard]] std::size_t instructionCount() const
        {
            const auto length = std::ranges::count_if(data, [](const auto& a) {
                return a.kind() != IR::Kind::Label;
            });

            if (length <= 0)
                return 0;
            return static_cast<std::size_t>(length);
        }
    };
}

#endif  // ARK_COMPILER_INTERMEDIATEREPRESENTATION_ENTITY_HPP
