#include <Ark/Compiler/IntermediateRepresentation/IROptimizer.hpp>

#include <cassert>
#include <utility>
#include <ranges>
#include <algorithm>

#include <Ark/Builtins/Builtins.hpp>

namespace Ark::internal
{
    IROptimizer::IROptimizer(const unsigned debug) :
        m_logger("IROptimizer", debug)
    {
        m_ruleset = {
            Rule { { LOAD_CONST, LOAD_CONST }, LOAD_CONST_LOAD_CONST },
            Rule { { LOAD_CONST, STORE }, LOAD_CONST_STORE },
            Rule { { LOAD_CONST, SET_VAL }, LOAD_CONST_SET_VAL },
            Rule { { LOAD_SYMBOL, STORE }, STORE_FROM },
            Rule { { LOAD_SYMBOL_BY_INDEX, STORE }, STORE_FROM_INDEX },
            Rule { { LOAD_SYMBOL, SET_VAL }, SET_VAL_FROM },
            Rule { { LOAD_SYMBOL_BY_INDEX, SET_VAL }, SET_VAL_FROM_INDEX },
            Rule { { STORE, PUSH_RETURN_ADDRESS, LOAD_SYMBOL_BY_INDEX, BUILTIN, CALL }, [](const Entities entities) {
                      return Builtins::builtins[entities[3].primaryArg()].second.isFunction();
                  },
                   [](const Entities e) {
                       return IR::Entity(CALL_BUILTIN_WITHOUT_RETURN_ADDRESS, e[3].primaryArg(), 1);
                   } },
            Rule { { STORE, STORE, PUSH_RETURN_ADDRESS, LOAD_SYMBOL_BY_INDEX, LOAD_SYMBOL_BY_INDEX, BUILTIN, CALL }, [](const Entities entities) {
                      return Builtins::builtins[entities[5].primaryArg()].second.isFunction();
                  },
                   [](const Entities e) {
                       return IR::Entity(CALL_BUILTIN_WITHOUT_RETURN_ADDRESS, e[5].primaryArg(), 2);
                   } },
            Rule { { STORE, STORE, STORE, PUSH_RETURN_ADDRESS, LOAD_SYMBOL_BY_INDEX, LOAD_SYMBOL_BY_INDEX, LOAD_SYMBOL_BY_INDEX, BUILTIN, CALL }, [](const Entities entities) {
                      return Builtins::builtins[entities[7].primaryArg()].second.isFunction();
                  },
                   [](const Entities e) {
                       return IR::Entity(CALL_BUILTIN_WITHOUT_RETURN_ADDRESS, e[7].primaryArg(), 3);
                   } },
            Rule { { BUILTIN, CALL }, CALL_BUILTIN, [](const Entities entities) {
                      return Builtins::builtins[entities[0].primaryArg()].second.isFunction();
                  } },
            Rule { { LOAD_SYMBOL, CALL }, CALL_SYMBOL },
            Rule { { GET_CURRENT_PAGE_ADDR, CALL }, CALL_CURRENT_PAGE },
            Rule { { LOAD_SYMBOL, GET_FIELD }, GET_FIELD_FROM_SYMBOL },
            Rule { { LOAD_SYMBOL_BY_INDEX, GET_FIELD }, GET_FIELD_FROM_SYMBOL_INDEX },
            Rule { { LIST, STORE }, STORE_LIST },
            Rule { { LOAD_SYMBOL, APPEND_IN_PLACE }, APPEND_IN_PLACE_SYM },
            Rule { { LOAD_SYMBOL_BY_INDEX, APPEND_IN_PLACE }, APPEND_IN_PLACE_SYM_INDEX },
            // LOAD_SYMBOL a / LOAD_SYMBOL_BY_INDEX index
            // LOAD_CONST n (1)
            // ADD / SUB
            // STORE
            // ---> INCREMENT_STORE / DECREMENT_STORE a value
            Rule { { LOAD_CONST, LOAD_SYMBOL, ADD, SET_VAL }, [this](const Entities e) {
                      return isPositiveNumberInlinable(e[0].primaryArg()) && e[1].primaryArg() == e[3].primaryArg();
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT_STORE, e[1].primaryArg(), numberAsArg(e[0].primaryArg()));
                   } },
            Rule { { LOAD_SYMBOL, LOAD_CONST, ADD, SET_VAL }, [this](const Entities e) {
                      return isPositiveNumberInlinable(e[1].primaryArg()) && e[0].primaryArg() == e[3].primaryArg();
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT_STORE, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_SYMBOL, LOAD_CONST, SUB, SET_VAL }, [this](const Entities e) {
                      return isPositiveNumberInlinable(e[1].primaryArg()) && e[0].primaryArg() == e[3].primaryArg();
                  },
                   [this](const Entities e) {
                       return IR::Entity(DECREMENT_STORE, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            // without the final store, just increment/decrement
            Rule { { LOAD_CONST, LOAD_SYMBOL, ADD }, [this](const Entities e) {
                      return isPositiveNumberInlinable(e[0].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT, e[1].primaryArg(), numberAsArg(e[0].primaryArg()));
                   } },
            Rule { { LOAD_SYMBOL, LOAD_CONST, ADD }, [this](const Entities e) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_SYMBOL, LOAD_CONST, SUB }, [this](const Entities e) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(DECREMENT, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_CONST, LOAD_SYMBOL_BY_INDEX, ADD }, [this](const Entities e) {
                      return isPositiveNumberInlinable(e[0].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT_BY_INDEX, e[1].primaryArg(), numberAsArg(e[0].primaryArg()));
                   } },
            Rule { { LOAD_SYMBOL_BY_INDEX, LOAD_CONST, ADD }, [this](const Entities e) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT_BY_INDEX, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_SYMBOL_BY_INDEX, LOAD_CONST, SUB }, [this](const Entities e) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(DECREMENT_BY_INDEX, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            // LOAD_SYMBOL list
            // TAIL / HEAD
            // STORE / SET_VAL a
            // ---> STORE_TAIL list a ; STORE_HEAD ; SET_VAL_TAIL ; SET_VAL_HEAD
            Rule { { LOAD_SYMBOL, TAIL, STORE }, [](const Entities e) {
                      return IR::Entity(STORE_TAIL, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL, TAIL, SET_VAL }, [](const Entities e) {
                      return IR::Entity(SET_VAL_TAIL, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL, HEAD, STORE }, [](const Entities e) {
                      return IR::Entity(STORE_HEAD, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL, HEAD, SET_VAL }, [](const Entities e) {
                      return IR::Entity(SET_VAL_HEAD, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL_BY_INDEX, TAIL, STORE }, [](const Entities e) {
                      return IR::Entity(STORE_TAIL_BY_INDEX, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL_BY_INDEX, TAIL, SET_VAL }, [](const Entities e) {
                      return IR::Entity(SET_VAL_TAIL_BY_INDEX, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL_BY_INDEX, HEAD, STORE }, [](const Entities e) {
                      return IR::Entity(STORE_HEAD_BY_INDEX, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL_BY_INDEX, HEAD, SET_VAL }, [](const Entities e) {
                      return IR::Entity(SET_VAL_HEAD_BY_INDEX, e[0].primaryArg(), e[2].primaryArg());
                  } },
            // LOAD_CONST id / LOAD_SYMBOL id
            // <comparison operator>
            // POP_JUMP_IF_(FALSE|TRUE)
            // ---> <OP>_(CONST|SYM)_JUMP_IF_(FALSE|TRUE)
            Rule { { LOAD_CONST, LT, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], LT_CONST_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
            Rule { { LOAD_CONST, LT, POP_JUMP_IF_TRUE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], LT_CONST_JUMP_IF_TRUE, e[0].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL, LT, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], LT_SYM_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
            Rule { { LOAD_CONST, GT, POP_JUMP_IF_TRUE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], GT_CONST_JUMP_IF_TRUE, e[0].primaryArg());
                  } },
            Rule { { LOAD_CONST, GT, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], GT_CONST_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL, GT, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], GT_SYM_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
            Rule { { LOAD_CONST, EQ, POP_JUMP_IF_TRUE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], EQ_CONST_JUMP_IF_TRUE, e[0].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL_BY_INDEX, EQ, POP_JUMP_IF_TRUE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], EQ_SYM_INDEX_JUMP_IF_TRUE, e[0].primaryArg());
                  } },
            Rule { { LOAD_CONST, NEQ, POP_JUMP_IF_TRUE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], NEQ_CONST_JUMP_IF_TRUE, e[0].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL, NEQ, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], NEQ_SYM_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
            // LOAD_SYMBOL id
            // LOAD_SYMBOL id2
            // AT
            // ---> AT_SYM_SYM id id2
            Rule { { LOAD_SYMBOL, LOAD_SYMBOL, AT }, AT_SYM_SYM },
            Rule { { LOAD_SYMBOL_BY_INDEX, LOAD_SYMBOL_BY_INDEX, AT }, AT_SYM_INDEX_SYM_INDEX },
            // LOAD_SYMBOL sym
            // TYPE
            // LOAD_CONST cst
            // EQ
            // ---> CHECK_TYPE_OF sym, cst
            // also works with LOAD_CONST cst, LOAD_SYMBOL sym, TYPE, EQ, but args will be flipped
            Rule { { LOAD_SYMBOL, TYPE, LOAD_CONST, EQ }, [](const Entities e) {
                      return IR::Entity(CHECK_TYPE_OF, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_CONST, LOAD_SYMBOL, TYPE, EQ }, [](const Entities e) {
                      return IR::Entity(CHECK_TYPE_OF, e[1].primaryArg(), e[0].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL_BY_INDEX, TYPE, LOAD_CONST, EQ }, [](const Entities e) {
                      return IR::Entity(CHECK_TYPE_OF_BY_INDEX, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_CONST, LOAD_SYMBOL_BY_INDEX, TYPE, EQ }, [](const Entities e) {
                      return IR::Entity(CHECK_TYPE_OF_BY_INDEX, e[1].primaryArg(), e[0].primaryArg());
                  } },
            // ---
            Rule { { LOAD_SYMBOL_BY_INDEX, LEN, STORE }, [](const Entities e) {
                      return IR::Entity(STORE_LEN, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_SYMBOL, LEN, LT, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[3], LT_LEN_SYM_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
        };
    }

    void IROptimizer::process(const std::vector<IR::Block>& pages, const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values)
    {
        m_logger.traceStart("process");
        m_symbols = symbols;
        m_values = values;

        for (const auto& block : pages)
        {
            m_ir.emplace_back();
            IR::Block& current_block = m_ir.back();

            std::size_t i = 0;
            const std::size_t end = block.size();

            while (i < end)
            {
                std::optional<EntityWithOffset> maybe_compacted = replaceWithRules(
                    m_ruleset,
                    std::span(
                        block.begin() + static_cast<IR::Block::difference_type>(i),
                        block.size() - i));

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

    bool IROptimizer::match(const std::vector<Instruction>& expected_insts, const std::span<const IR::Entity> entities) const
    {
        if (expected_insts.size() > entities.size())
            return false;

        for (std::size_t i = 0; i < expected_insts.size(); ++i)
        {
            if (expected_insts[i] != entities[i].inst())
                return false;
        }

        return true;
    }

    bool IROptimizer::canBeOptimizedSafely(std::span<const IR::Entity> entities, std::size_t window_size) const
    {
        // check that we can actually safely apply the optimization on the given instructions
        return std::ranges::none_of(
            entities | std::ranges::views::take(window_size),
            [](const IR::Entity& entity) {
                return entity.primaryArg() > IR::MaxValueForDualArg;
            });
    }

    std::optional<EntityWithOffset> IROptimizer::replaceWithRules(const std::vector<Rule>& rules, const std::span<const IR::Entity> entities)
    {
        for (const auto& [expected, condition, createReplacement] : rules)
        {
            if (match(expected, entities) && condition(entities))
            {
                const std::size_t window_size = expected.size();
                if (!canBeOptimizedSafely(entities, window_size))
                    return std::nullopt;  // no need to try other optimizations, they won't be applied either

                auto output = createReplacement(entities);

                if (const auto it = std::ranges::find_if(entities, [](const auto& entity) {
                        return entity.hasValidSourceLocation();
                    });
                    it != entities.end())
                    output.setSourceLocation(it->filename(), it->sourceLine());

                return EntityWithOffset { output, window_size };
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
