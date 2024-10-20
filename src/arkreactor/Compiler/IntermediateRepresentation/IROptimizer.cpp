#include <Ark/Compiler/IntermediateRepresentation/IROptimizer.hpp>

#include <utility>
#include <Ark/Builtins/Builtins.hpp>

namespace Ark::internal
{
    struct EntityWithOffset
    {
        IR::Entity entity;
        std::size_t offset;
    };

    IROptimizer::IROptimizer(const unsigned debug) :
        m_logger("IROptimizer", debug)
    {}

    void IROptimizer::process(const std::vector<IR::Block>& pages, const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values)
    {
        m_symbols = symbols;
        m_values = values;

        auto map = []<typename T>(const std::optional<T>& opt, auto&& lambda) -> decltype(std::optional(lambda(opt.value()))) {
            if (opt.has_value())
                return lambda(opt.value());
            return std::nullopt;
        };

        auto or_else = []<typename T>(const std::optional<T>& opt, auto&& lambda) -> std::optional<T> {
            if (!opt.has_value())
                return lambda();
            return opt;
        };

        for (const auto& block : pages)
        {
            m_ir.emplace_back();
            IR::Block& current_block = m_ir.back();

            std::size_t i = 0;
            const std::size_t end = block.size();

            while (i < end)
            {
                std::optional<EntityWithOffset> maybe_compacted = std::nullopt;

                if (i + 1 < end)
                    maybe_compacted = map(
                        compactEntities(block[i], block[i + 1]),
                        [](const auto& entity) {
                            return std::make_optional<EntityWithOffset>(entity, 2);
                        });
                if (i + 2 < end)
                    maybe_compacted = or_else(
                        maybe_compacted,
                        [&, this]() {
                            return map(
                                compactEntities(block[i], block[i + 1], block[i + 2]),
                                [](const auto& entity) {
                                    return std::make_optional<EntityWithOffset>(entity, 3);
                                });
                        });

                if (maybe_compacted.has_value())
                {
                    auto [entity, offset] = maybe_compacted.value();
                    current_block.emplace_back(entity);
                    i += offset;
                }
                else
                {
                    current_block.emplace_back(block[i]);
                    ++i;
                }
            }
        }
    }

    const std::vector<IR::Block>& IROptimizer::intermediateRepresentation() const noexcept
    {
        return m_ir;
    }

    std::optional<IR::Entity> IROptimizer::compactEntities(const IR::Entity& first, const IR::Entity& second)
    {
        if (first.primaryArg() > IR::MaxValueForDualArg || second.primaryArg() > IR::MaxValueForDualArg)
            return std::nullopt;

        // LOAD_CONST x
        // LOAD_CONST y
        // ---> LOAD_CONST_LOAD_CONST x y
        if (first.inst() == LOAD_CONST && second.inst() == LOAD_CONST)
            return IR::Entity(LOAD_CONST_LOAD_CONST, first.primaryArg(), second.primaryArg());
        // LOAD_CONST x
        // STORE / SET_VAL a
        // ---> LOAD_CONST_STORE x a ; LOAD_CONST_SET_VAL x a
        if (first.inst() == LOAD_CONST && second.inst() == STORE)
            return IR::Entity(LOAD_CONST_STORE, first.primaryArg(), second.primaryArg());
        if (first.inst() == LOAD_CONST && second.inst() == SET_VAL)
            return IR::Entity(LOAD_CONST_SET_VAL, first.primaryArg(), second.primaryArg());
        // LOAD_SYMBOL a
        // STORE / SET_VAL b
        // ---> STORE_FROM a b ; SET_VAL_FROM a b
        if (first.inst() == LOAD_SYMBOL && second.inst() == STORE)
            return IR::Entity(STORE_FROM, first.primaryArg(), second.primaryArg());
        if (first.inst() == LOAD_SYMBOL && second.inst() == SET_VAL)
            return IR::Entity(SET_VAL_FROM, first.primaryArg(), second.primaryArg());
        // BUILTIN i
        // CALL n
        // ---> CALL_BUILTIN i n
        if (first.inst() == BUILTIN && second.inst() == CALL && Builtins::builtins[first.primaryArg()].second.isFunction())
            return IR::Entity(CALL_BUILTIN, first.primaryArg(), second.primaryArg());

        return std::nullopt;
    }

    std::optional<IR::Entity> IROptimizer::compactEntities(const IR::Entity& first, const IR::Entity& second, const IR::Entity& third)
    {
        if (first.primaryArg() > IR::MaxValueForDualArg || second.primaryArg() > IR::MaxValueForDualArg || third.primaryArg() > IR::MaxValueForDualArg)
            return std::nullopt;

        // LOAD_SYMBOL a
        // LOAD_CONST n (1)
        // ADD / SUB
        // ---> INCREMENT / DECREMENT a
        if (third.inst() == ADD && first.inst() == LOAD_CONST && second.inst() == LOAD_SYMBOL && isNumber(first.primaryArg(), 1))
            return IR::Entity(INCREMENT, second.primaryArg());
        if (third.inst() == ADD && first.inst() == LOAD_SYMBOL && second.inst() == LOAD_CONST && isNumber(second.primaryArg(), 1))
            return IR::Entity(INCREMENT, first.primaryArg());
        if (third.inst() == SUB && first.inst() == LOAD_SYMBOL && second.inst() == LOAD_CONST && isNumber(second.primaryArg(), 1))
            return IR::Entity(DECREMENT, first.primaryArg());
        // LOAD_SYMBOL list
        // TAIL / HEAD
        // STORE / SET_VAL a
        // ---> STORE_TAIL list a ; STORE_HEAD ; SET_VAL_TAIL ; SET_VAL_HEAD
        if (first.inst() == LOAD_SYMBOL && second.inst() == TAIL && third.inst() == STORE)
            return IR::Entity(STORE_TAIL, first.primaryArg(), third.primaryArg());
        if (first.inst() == LOAD_SYMBOL && second.inst() == TAIL && third.inst() == SET_VAL)
            return IR::Entity(SET_VAL_TAIL, first.primaryArg(), third.primaryArg());
        if (first.inst() == LOAD_SYMBOL && second.inst() == HEAD && third.inst() == STORE)
            return IR::Entity(STORE_HEAD, first.primaryArg(), third.primaryArg());
        if (first.inst() == LOAD_SYMBOL && second.inst() == HEAD && third.inst() == SET_VAL)
            return IR::Entity(SET_VAL_HEAD, first.primaryArg(), third.primaryArg());

        return std::nullopt;
    }

    bool IROptimizer::isNumber(const uint16_t id, const double expected_number) const
    {
        return std::cmp_less(id, m_values.size()) && m_values[id].type == ValTableElemType::Number && std::get<double>(m_values[id].value) == expected_number;
    }
}
