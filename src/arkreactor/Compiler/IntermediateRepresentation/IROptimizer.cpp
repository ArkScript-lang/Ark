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
    {
        m_ruleset_two = {
            Rule {
                { LOAD_CONST, LOAD_CONST }, LOAD_CONST_LOAD_CONST },
            Rule {
                { LOAD_CONST, STORE }, LOAD_CONST_STORE },
            Rule {
                { LOAD_CONST, SET_VAL }, LOAD_CONST_SET_VAL },
            Rule {
                { LOAD_SYMBOL, STORE }, STORE_FROM },
            Rule {
                { LOAD_SYMBOL_BY_INDEX, STORE }, STORE_FROM_INDEX },
            Rule {
                { LOAD_SYMBOL, SET_VAL }, SET_VAL_FROM },
            Rule {
                { LOAD_SYMBOL_BY_INDEX, SET_VAL }, SET_VAL_FROM_INDEX },
            Rule {
                { BUILTIN, CALL }, CALL_BUILTIN, [](const Entities& entities) {
                    return Builtins::builtins[entities[0].primaryArg()].second.isFunction();
                } }
        };

        m_ruleset_three = {
            // LOAD_SYMBOL a / LOAD_SYMBOL_BY_INDEX index
            // LOAD_CONST n (1)
            // ADD / SUB
            // ---> INCREMENT / DECREMENT a value
            Rule {
                { LOAD_CONST, LOAD_SYMBOL, ADD }, INCREMENT, [this](const Entities& e) {
                    return isPositiveNumberInlinable(e[0].primaryArg());
                },
                [this](const Entities& e) {
                    return std::make_pair(e[1].primaryArg(), numberAsArg(e[0].primaryArg()));
                } },
            Rule { { LOAD_SYMBOL, LOAD_CONST, ADD }, INCREMENT, [this](const Entities& e) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities& e) {
                       return std::make_pair(e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_SYMBOL, LOAD_CONST, SUB }, DECREMENT, [this](const Entities& e) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities& e) {
                       return std::make_pair(e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_CONST, LOAD_SYMBOL_BY_INDEX, ADD }, INCREMENT_BY_INDEX, [this](const Entities& e) {
                      return isPositiveNumberInlinable(e[0].primaryArg());
                  },
                   [this](const Entities& e) {
                       return std::make_pair(e[1].primaryArg(), numberAsArg(e[0].primaryArg()));
                   } },
            Rule { { LOAD_SYMBOL_BY_INDEX, LOAD_CONST, ADD }, INCREMENT_BY_INDEX, [this](const Entities& e) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities& e) {
                       return std::make_pair(e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_SYMBOL_BY_INDEX, LOAD_CONST, SUB }, DECREMENT_BY_INDEX, [this](const Entities& e) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities& e) {
                       return std::make_pair(e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            // LOAD_SYMBOL list
            // TAIL / HEAD
            // STORE / SET_VAL a
            // ---> STORE_TAIL list a ; STORE_HEAD ; SET_VAL_TAIL ; SET_VAL_HEAD
            Rule { .expected = { LOAD_SYMBOL, TAIL, STORE }, .replacement = STORE_TAIL, .createReplacement = [](const Entities& e) {
                      return std::make_pair(e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { .expected = { LOAD_SYMBOL, TAIL, SET_VAL }, .replacement = SET_VAL_TAIL, .createReplacement = [](const Entities& e) {
                      return std::make_pair(e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { .expected = { LOAD_SYMBOL, HEAD, STORE }, .replacement = STORE_HEAD, .createReplacement = [](const Entities& e) {
                      return std::make_pair(e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { .expected = { LOAD_SYMBOL, HEAD, SET_VAL }, .replacement = SET_VAL_HEAD, .createReplacement = [](const Entities& e) {
                      return std::make_pair(e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { .expected = { LOAD_SYMBOL_BY_INDEX, TAIL, STORE }, .replacement = STORE_TAIL_BY_INDEX, .createReplacement = [](const Entities& e) {
                      return std::make_pair(e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { .expected = { LOAD_SYMBOL_BY_INDEX, TAIL, SET_VAL }, .replacement = SET_VAL_TAIL_BY_INDEX, .createReplacement = [](const Entities& e) {
                      return std::make_pair(e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { .expected = { LOAD_SYMBOL_BY_INDEX, HEAD, STORE }, .replacement = STORE_HEAD_BY_INDEX, .createReplacement = [](const Entities& e) {
                      return std::make_pair(e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { .expected = { LOAD_SYMBOL_BY_INDEX, HEAD, SET_VAL }, .replacement = SET_VAL_HEAD_BY_INDEX, .createReplacement = [](const Entities& e) {
                      return std::make_pair(e[0].primaryArg(), e[2].primaryArg());
                  } }
        };
    }

    void IROptimizer::process(const std::vector<IR::Block>& pages, const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values)
    {
        m_logger.traceStart("process");
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
                        replaceWithRules(m_ruleset_two, { block[i], block[i + 1] }),
                        [](const auto& entity) {
                            return std::make_optional<EntityWithOffset>(entity, 2);
                        });
                if (i + 2 < end)
                    maybe_compacted = or_else(
                        maybe_compacted,
                        [&, this]() {
                            return map(
                                replaceWithRules(m_ruleset_three, { block[i], block[i + 1], block[i + 2] }),
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

        m_logger.traceEnd();
    }

    const std::vector<IR::Block>& IROptimizer::intermediateRepresentation() const noexcept
    {
        return m_ir;
    }

    bool IROptimizer::match(const std::vector<Instruction>& expected_insts, const Entities& entities) const
    {
        assert(expected_insts.size() == entities.size() && "Mismatching size between expected instructions and given entities");

        for (std::size_t i = 0; i < expected_insts.size(); ++i)
        {
            if (expected_insts[i] != entities[i].inst())
                return false;
        }

        return true;
    }

    std::optional<IR::Entity> IROptimizer::replaceWithRules(const std::vector<Rule>& rules, const Entities& entities)
    {
        for (auto&& entity : entities)
        {
            if (entity.primaryArg() > IR::MaxValueForDualArg)
                return std::nullopt;
        }

        for (const auto& [expected, replacement, condition, createReplacement] : rules)
        {
            if (match(expected, entities) && condition(entities))
            {
                auto [first, second] = createReplacement(entities);
                return IR::Entity(replacement, first, second);
            }
        }

        return std::nullopt;
    }

    bool IROptimizer::isPositiveNumberInlinable(const uint16_t id) const
    {
        if (std::cmp_less(id, m_values.size()) && m_values[id].type == ValTableElemType::Number)
        {
            const double val = std::get<double>(m_values[id].value);
            return val >= 0.0 &&
                val < IR::MaxValueForDualArg &&
                static_cast<double>(static_cast<long>(val)) == val;
        }
        return false;
    }

    uint16_t IROptimizer::numberAsArg(const uint16_t id) const
    {
        return static_cast<uint16_t>(std::get<double>(m_values[id].value));
    }
}
