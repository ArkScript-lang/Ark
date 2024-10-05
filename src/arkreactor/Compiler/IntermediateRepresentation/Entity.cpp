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
        // todo: handle secondary_arg
        return Word(m_inst, m_primary_arg + m_secondary_arg);
    }
}
