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

    Entity Entity::Label()
    {
        auto label = Entity(Kind::Label);
        label.m_label = Entity::LabelCounter++;

        return label;
    }

    Entity Entity::Goto(const Entity& label)
    {
        auto jump = Entity(Kind::Goto);
        jump.m_label = label.m_label;

        return jump;
    }

    Entity Entity::GotoIf(const Entity& label, const bool cond)
    {
        auto jump = Entity(cond ? Kind::GotoIfTrue : Kind::GotoIfFalse);
        jump.m_label = label.m_label;

        return jump;
    }

    Word Entity::bytecode() const
    {
        if (m_kind == Kind::Opcode)
            return Word(m_inst, m_primary_arg);
        if (m_kind == Kind::Opcode2Args)
            return Word(m_inst, m_primary_arg, m_secondary_arg);
        return Word(0, 0);
    }
}
