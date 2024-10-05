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
        Opcode
    };

    using label_t = std::size_t;

    class Entity
    {
    public:
        explicit Entity(Kind kind);

        explicit Entity(Instruction inst, uint16_t arg = 0);

        static Entity Label();

        static Entity Goto(const Entity& label);

        static Entity GotoIf(const Entity& label, bool cond);

        [[nodiscard]] Word bytecode() const;

        [[nodiscard]] inline label_t label() const { return m_label; }

        [[nodiscard]] inline Kind kind() const { return m_kind; }

    private:
        inline static label_t LabelCounter = 0;

        Kind m_kind;
        label_t m_label { 0 };
        Instruction m_inst;
        uint8_t m_secondary_arg { 0 };
        uint16_t m_primary_arg { 0 };
    };

    using Block = std::vector<Entity>;
}

#endif  // ARK_COMPILER_INTERMEDIATEREPRESENTATION_ENTITY_HPP
