#include <Ark/Compiler/IntermediateRepresentation/Entity.hpp>

namespace Ark::internal::IR
{
    Entity::Entity(const Kind kind) :
        m_kind(kind),
        m_inst(NOP)
    {}

    Entity::Entity(const Instruction inst, const uint16_t arg) :
        m_kind(Kind::Opcode),
        m_inst(inst), m_primary_arg(arg)
    {}

    Entity::Entity(const Instruction inst, const uint16_t primary_arg, const uint16_t secondary_arg) :
        m_kind(Kind::Opcode2Args),
        m_inst(inst), m_primary_arg(primary_arg), m_secondary_arg(secondary_arg)
    {}

    Entity::Entity(const Instruction inst, const uint8_t inst2, const uint8_t inst3, const uint8_t inst4) :
        m_kind(Kind::Opcode3Args),
        m_inst(inst), m_primary_arg(inst2), m_secondary_arg(inst3), m_tertiary_arg(inst4)
    {}

    void Entity::replaceInstruction(const Instruction replacement)
    {
        m_inst = replacement;
    }

    void Entity::replaceLabel(const label_t replacement)
    {
        m_label = replacement;
    }

    Entity Entity::Label(const label_t value)
    {
        auto entity = Entity(Kind::Label);
        entity.m_label = value;

        return entity;
    }

    Entity Entity::Goto(const Entity& label, const Instruction inst)
    {
        auto jump = Entity(Kind::Goto);
        jump.m_label = label.m_label;
        jump.m_inst = inst;

        return jump;
    }

    Entity Entity::GotoWithArg(const Entity& label, const Instruction inst, const uint16_t primary_arg)
    {
        auto jump = Entity(Kind::GotoWithArg);
        jump.m_label = label.m_label;
        jump.m_inst = inst;
        jump.m_primary_arg = primary_arg;

        return jump;
    }

    Entity Entity::GotoIf(const Entity& label, const bool cond)
    {
        return Goto(label, cond ? POP_JUMP_IF_TRUE : POP_JUMP_IF_FALSE);
    }

    Word Entity::bytecode() const
    {
        if (m_kind == Kind::Opcode)
            return Word(m_inst, m_primary_arg);
        if (m_kind == Kind::Opcode2Args)
            return Word(m_inst, m_primary_arg, m_secondary_arg);
        if (m_kind == Kind::Opcode3Args)
            return Word(
                m_inst,
                static_cast<uint8_t>(m_primary_arg),
                static_cast<uint8_t>(m_secondary_arg),
                static_cast<uint8_t>(m_tertiary_arg));
        return Word(0, 0);
    }

    void Entity::setSourceLocation(const std::string& filename, const std::size_t line)
    {
        m_metadata.source_file = filename;
        m_metadata.source_line = line;
    }

    void Entity::setRelatedResourceId(const std::optional<uint16_t> id)
    {
        m_metadata.related_res_id = id;
    }
}
