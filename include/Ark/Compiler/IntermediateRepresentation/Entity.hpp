/**
 * @file Entity.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief An entity in the IR is a bundle of information
 * @version 0.1
 * @date 2024-10-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef ARK_COMPILER_INTERMEDIATEREPRESENTATION_ENTITY_HPP
#define ARK_COMPILER_INTERMEDIATEREPRESENTATION_ENTITY_HPP

#include <cinttypes>
#include <vector>

#include <Ark/Compiler/Word.hpp>
#include <Ark/Compiler/Instructions.hpp>

namespace Ark::internal::IR
{
    enum class Kind
    {
        Label,
        Goto,
        GotoIfTrue,
        GotoIfFalse,
        Opcode,
        Opcode2Args
    };

    using label_t = std::size_t;

    /// The maximum value an argument can have when an IR entity has two arguments
    constexpr uint16_t MaxValueForDualArg = 0x0fff;

    class Entity
    {
    public:
        explicit Entity(Kind kind);

        explicit Entity(Instruction inst, uint16_t arg = 0);

        Entity(Instruction inst, uint16_t primary_arg, uint16_t secondary_arg);

        static Entity Label();

        static Entity Goto(const Entity& label);

        static Entity GotoIf(const Entity& label, bool cond);

        [[nodiscard]] Word bytecode() const;

        [[nodiscard]] inline label_t label() const { return m_label; }

        [[nodiscard]] inline Kind kind() const { return m_kind; }

        [[nodiscard]] inline Instruction inst() const { return m_inst; }

        [[nodiscard]] inline uint16_t primaryArg() const { return m_primary_arg; }

        [[nodiscard]] inline uint16_t secondaryArg() const { return m_secondary_arg; }

    private:
        inline static label_t LabelCounter = 0;

        Kind m_kind;
        label_t m_label { 0 };
        Instruction m_inst { NOP };
        uint16_t m_primary_arg { 0 };
        uint16_t m_secondary_arg { 0 };
    };

    using Block = std::vector<Entity>;
}

#endif  // ARK_COMPILER_INTERMEDIATEREPRESENTATION_ENTITY_HPP
