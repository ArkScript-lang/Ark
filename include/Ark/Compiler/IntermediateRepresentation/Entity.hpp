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
#include <algorithm>

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

    constexpr std::string_view AnonymousBlockName = "#anonymous";

    class Entity
    {
    public:
        /**
         * @brief Create a new IR Entity
         *
         * @param kind kind of entity (label, jump, instruction...)
         */
        explicit Entity(Kind kind);

        /**
         * @brief Create a new IR Entity
         *
         * @param inst instruction
         * @param arg optional argument, default to 0
         */
        explicit Entity(Instruction inst, uint16_t arg = 0);

        /**
         * @brief Create a new IR Entity
         *
         * @param inst instruction that takes two arguments
         * @param primary_arg first argument on 12 bits
         * @param secondary_arg second argument on 12 bits
         */
        Entity(Instruction inst, uint16_t primary_arg, uint16_t secondary_arg);

        /**
         * @brief Create a new IR Entity
         *
         * @param inst instruction that takes three arguments
         * @param inst2 first argument on 8 bits
         * @param inst3 second argument on 8 bits
         * @param inst4 third argument on 8 bits
         */
        Entity(Instruction inst, uint8_t inst2, uint8_t inst3, uint8_t inst4);

        void replaceInstruction(Instruction replacement);

        void replaceLabel(label_t replacement);

        /**
         * @brief Create a new Label IR Entity
         *
         * @param value value for the label
         * @return Entity
         */
        static Entity Label(label_t value);

        /**
         * @brief Create a new Goto IR Entity
         *
         * @param label label the goto relates to
         * @param inst jump instruction to use, default to JUMP
         * @return Entity
         */
        static Entity Goto(const Entity& label, Instruction inst = Instruction::JUMP);

        /**
         * @brief Create a new Goto IR Entity
         *
         * @param label label the goto relates to
         * @param inst jump instruction to use
         * @param primary_arg argument for the jump instruction
         * @return Entity
         */
        static Entity GotoWithArg(const Entity& label, Instruction inst, uint16_t primary_arg);

        /**
         * @brief Create a new Goto IR Entity
         *
         * @param label label the goto relates to
         * @param cond true to use POP_JUMP_IF_TRUE, false to use POP_JUMP_IF_FALSE
         * @return Entity
         */
        static Entity GotoIf(const Entity& label, bool cond);

        /**
         * @brief Return the bytecode representation of the IR Entity if it's an Opcode
         *
         * @return Word
         */
        [[nodiscard]] Word bytecode() const;

        /**
         * @brief Check if the Entity has a label attached
         *
         * @return bool
         */
        [[nodiscard]] bool hasLabel() const
        {
            switch (m_kind)
            {
                case Kind::Label:
                    [[fallthrough]];
                case Kind::Goto:
                    [[fallthrough]];
                case Kind::GotoWithArg:
                    return true;

                case Kind::Opcode:
                    [[fallthrough]];
                case Kind::Opcode2Args:
                    [[fallthrough]];
                case Kind::Opcode3Args:
                    return false;
            }

            return false;
        }

        /**
         * @brief Return the label of the IR Entity
         *
         * @return label_t
         */
        [[nodiscard]] label_t label() const { return m_label; }

        /**
         * @brief Return the kind of IR Entity
         * @see Kind
         * @return Kind
         */
        [[nodiscard]] Kind kind() const { return m_kind; }

        /**
         * @brief Return the underlying instruction of the IR Entity
         *
         * @return Instruction
         */
        [[nodiscard]] Instruction inst() const { return m_inst; }

        /**
         * @brief Return the primary argument of the IR Entity (can be 0 if the argument isn't used)
         * @details The argument is on 16 bits for standard instructions ; for super instructions, only the first 12 bits are used
         * @return uint16_t
         */
        [[nodiscard]] uint16_t primaryArg() const { return m_primary_arg; }

        /**
         * @brief Return the second argument of the IR Entity
         * @details The argument is for super instructions, where only the first 12 bits are used
         * @return uint16_t
         */
        [[nodiscard]] uint16_t secondaryArg() const { return m_secondary_arg; }

        /**
         * @brief Return the third argument of the IR Entity
         * @details The argument is for special super instructions, where only the first 8 bits are used (for all arguments)
         * @return uint16_t
         */
        [[nodiscard]] uint16_t tertiaryArg() const { return m_tertiary_arg; }

        /**
         * @brief Set the source location for an IR Entity, which is used to generate the file loc table
         *
         * @param filename
         * @param line
         */
        void setSourceLocation(const std::string& filename, std::size_t line);

        void setRelatedResourceId(std::optional<uint16_t> id);

        [[nodiscard]] bool hasValidSourceLocation() const { return !m_metadata.source_file.empty(); }

        [[nodiscard]] const std::string& filename() const { return m_metadata.source_file; }

        [[nodiscard]] std::size_t sourceLine() const { return m_metadata.source_line; }

        /**
         * @brief Return the related constant/symbol id an IR Entity refers to (only populated for LOAD_FAST_BY_INDEX, CALL_SYMBOL_BY_INDEX, CALL_SYMBOL, and CALL)
         *
         * @return std::optional<uint16_t>
         */
        [[nodiscard]] std::optional<uint16_t> relatedResourceId() const { return m_metadata.related_res_id; }

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
            std::optional<uint16_t> related_res_id;  ///< Used by a few instructions to know the original symbol/constant id and deoptimize when necessary
        } m_metadata;
    };

    /**
     * @brief Block of IR entities, with attached metadata
     */
    struct Block
    {
        using vec_t = std::vector<Entity>;

        struct Metadata
        {
            std::optional<std::string> name;
            std::size_t argument_count { 0 };
            std::size_t addr { 0 };
            bool is_closure { false };
            bool is_recursive { false };
            bool is_simple { false };  ///< Calls only builtin and operators, no user functions/C++ functions
            bool is_mutating_args { false };
        } metadata;
        vec_t data;

        [[nodiscard]] std::string debugName() const
        {
            return metadata.name.value_or(std::string(AnonymousBlockName));
        }

        [[nodiscard]] std::string metadataRepr() const
        {
            std::string flags;
            if (metadata.is_recursive)
                flags += "recursive";
            if (metadata.is_simple)
                flags += std::string(flags.empty() ? "" : " ") + "simple";
            if (metadata.is_mutating_args)
                flags += std::string(flags.empty() ? "" : " ") + "mutating";

            if (metadata.is_closure)
                flags += std::string(flags.empty() ? "" : " ") + "closure";
            else
                flags += std::string(flags.empty() ? "" : " ") + "function";
            return flags;
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
