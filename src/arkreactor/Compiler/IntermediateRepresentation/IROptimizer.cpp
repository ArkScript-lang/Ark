#include <Ark/Compiler/IntermediateRepresentation/IROptimizer.hpp>

#include <cassert>
#include <utility>
#include <ranges>
#include <algorithm>

#include <Ark/Builtins/Builtins.hpp>

namespace Ark::internal
{
    IR::Entity fuseMathOps3(const std::span<const IR::Entity> e)
    {
        return IR::Entity(FUSED_MATH, e[0].inst(), e[1].inst(), e[2].inst());
    }

    IR::Entity fuseMathOps2(const std::span<const IR::Entity> e)
    {
        return IR::Entity(FUSED_MATH, e[0].inst(), e[1].inst(), NOP);
    }

    IROptimizer::IROptimizer(const unsigned debug) :
        m_logger("IROptimizer", debug)
    {
        m_ruleset = {
            Rule { { LOAD_CONST, LOAD_CONST }, LOAD_CONST_LOAD_CONST },
            Rule { { LOAD_CONST, STORE }, LOAD_CONST_STORE },
            Rule { { LOAD_CONST, SET_VAL }, LOAD_CONST_SET_VAL },
            Rule { { LOAD_FAST, STORE }, STORE_FROM },
            Rule { { LOAD_FAST_BY_INDEX, STORE }, STORE_FROM_INDEX },
            Rule { { LOAD_FAST, SET_VAL }, SET_VAL_FROM },
            Rule { { LOAD_FAST_BY_INDEX, SET_VAL }, SET_VAL_FROM_INDEX },
            Rule { { STORE, PUSH_RETURN_ADDRESS, LOAD_FAST_BY_INDEX, BUILTIN, CALL },
                   [](const Entities entities, const std::size_t start_idx) {
                       return Builtins::builtins[entities[3].primaryArg()].second.isFunction() && start_idx == 0;
                   },
                   [](const Entities e) {
                       return IR::Entity(CALL_BUILTIN_WITHOUT_RETURN_ADDRESS, e[3].primaryArg(), 1);
                   } },
            Rule { { STORE, STORE, PUSH_RETURN_ADDRESS, LOAD_FAST_BY_INDEX, LOAD_FAST_BY_INDEX, BUILTIN, CALL },
                   [](const Entities entities, const std::size_t start_idx) {
                       return Builtins::builtins[entities[5].primaryArg()].second.isFunction() && start_idx == 0;
                   },
                   [](const Entities e) {
                       return IR::Entity(CALL_BUILTIN_WITHOUT_RETURN_ADDRESS, e[5].primaryArg(), 2);
                   } },
            Rule { { STORE, STORE, STORE, PUSH_RETURN_ADDRESS, LOAD_FAST_BY_INDEX, LOAD_FAST_BY_INDEX, LOAD_FAST_BY_INDEX, BUILTIN, CALL },
                   [](const Entities entities, const std::size_t start_idx) {
                       return Builtins::builtins[entities[7].primaryArg()].second.isFunction() && start_idx == 0;
                   },
                   [](const Entities e) {
                       return IR::Entity(CALL_BUILTIN_WITHOUT_RETURN_ADDRESS, e[7].primaryArg(), 3);
                   } },
            Rule { { BUILTIN, CALL }, CALL_BUILTIN, [](const Entities entities, const std::size_t) {
                      return Builtins::builtins[entities[0].primaryArg()].second.isFunction();
                  } },
            Rule { { LOAD_FAST, CALL }, CALL_SYMBOL },
            Rule { { GET_CURRENT_PAGE_ADDR, CALL }, CALL_CURRENT_PAGE },
            Rule { { LOAD_FAST, GET_FIELD }, GET_FIELD_FROM_SYMBOL },
            Rule { { LOAD_FAST_BY_INDEX, GET_FIELD }, GET_FIELD_FROM_SYMBOL_INDEX },
            Rule { { LIST, STORE }, STORE_LIST },
            Rule { { LOAD_FAST, APPEND_IN_PLACE }, APPEND_IN_PLACE_SYM },
            Rule { { LOAD_FAST_BY_INDEX, APPEND_IN_PLACE }, APPEND_IN_PLACE_SYM_INDEX },
            // LOAD_CONST, LOAD_FAST a, MUL, SET_VAL / LOAD_FAST a, LOAD_CONST, MUL, SET_VAL
            // ---> MUL_SET_VAL a value
            Rule { { LOAD_CONST, LOAD_FAST, MUL, SET_VAL }, [this](const Entities e, const std::size_t) {
                      return isSmallerNumberInlinable(e[0].primaryArg()) && e[1].primaryArg() == e[3].primaryArg();
                  },
                   [this](const Entities e) {
                       return IR::Entity(MUL_SET_VAL, e[1].primaryArg(), smallerNumberAsArg(e[0].primaryArg()));
                   } },
            Rule { { LOAD_FAST, LOAD_CONST, MUL, SET_VAL }, [this](const Entities e, const std::size_t) {
                      return isSmallerNumberInlinable(e[1].primaryArg()) && e[0].primaryArg() == e[3].primaryArg();
                  },
                   [this](const Entities e) {
                       return IR::Entity(MUL_SET_VAL, e[0].primaryArg(), smallerNumberAsArg(e[1].primaryArg()));
                   } },
            // LOAD_CONST, LOAD_FAST a, MUL / LOAD_FAST a, LOAD_CONST, MUL
            // ---> MUL_(BY|BY_INDEX) a value
            Rule { { LOAD_CONST, LOAD_FAST, MUL }, [this](const Entities e, const std::size_t) {
                      return isSmallerNumberInlinable(e[0].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(MUL_BY, e[1].primaryArg(), smallerNumberAsArg(e[0].primaryArg()));
                   } },
            Rule { { LOAD_FAST, LOAD_CONST, MUL }, [this](const Entities e, const std::size_t) {
                      return isSmallerNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(MUL_BY, e[0].primaryArg(), smallerNumberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_CONST, LOAD_FAST_BY_INDEX, MUL }, [this](const Entities e, const std::size_t) {
                      return isSmallerNumberInlinable(e[0].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(MUL_BY_INDEX, e[1].primaryArg(), smallerNumberAsArg(e[0].primaryArg()));
                   } },
            Rule { { LOAD_FAST_BY_INDEX, LOAD_CONST, MUL }, [this](const Entities e, const std::size_t) {
                      return isSmallerNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(MUL_BY_INDEX, e[0].primaryArg(), smallerNumberAsArg(e[1].primaryArg()));
                   } },
            // (LOAD_FAST a | LOAD_FAST_BY_INDEX index), LOAD_CONST n (=1), (ADD | SUB), STORE
            // ---> INCREMENT_STORE / DECREMENT_STORE a value
            Rule { { LOAD_CONST, LOAD_FAST, ADD, SET_VAL }, [this](const Entities e, const std::size_t) {
                      return isPositiveNumberInlinable(e[0].primaryArg()) && e[1].primaryArg() == e[3].primaryArg();
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT_STORE, e[1].primaryArg(), numberAsArg(e[0].primaryArg()));
                   } },
            Rule { { LOAD_FAST, LOAD_CONST, ADD, SET_VAL }, [this](const Entities e, const std::size_t) {
                      return isPositiveNumberInlinable(e[1].primaryArg()) && e[0].primaryArg() == e[3].primaryArg();
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT_STORE, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_FAST, LOAD_CONST, SUB, SET_VAL }, [this](const Entities e, const std::size_t) {
                      return isPositiveNumberInlinable(e[1].primaryArg()) && e[0].primaryArg() == e[3].primaryArg();
                  },
                   [this](const Entities e) {
                       return IR::Entity(DECREMENT_STORE, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            // without the final store, just increment/decrement
            Rule { { LOAD_CONST, LOAD_FAST, ADD }, [this](const Entities e, const std::size_t) {
                      return isPositiveNumberInlinable(e[0].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT, e[1].primaryArg(), numberAsArg(e[0].primaryArg()));
                   } },
            Rule { { LOAD_FAST, LOAD_CONST, ADD }, [this](const Entities e, const std::size_t) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_FAST, LOAD_CONST, SUB }, [this](const Entities e, const std::size_t) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(DECREMENT, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_CONST, LOAD_FAST_BY_INDEX, ADD }, [this](const Entities e, const std::size_t) {
                      return isPositiveNumberInlinable(e[0].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT_BY_INDEX, e[1].primaryArg(), numberAsArg(e[0].primaryArg()));
                   } },
            Rule { { LOAD_FAST_BY_INDEX, LOAD_CONST, ADD }, [this](const Entities e, const std::size_t) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(INCREMENT_BY_INDEX, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            Rule { { LOAD_FAST_BY_INDEX, LOAD_CONST, SUB }, [this](const Entities e, const std::size_t) {
                      return isPositiveNumberInlinable(e[1].primaryArg());
                  },
                   [this](const Entities e) {
                       return IR::Entity(DECREMENT_BY_INDEX, e[0].primaryArg(), numberAsArg(e[1].primaryArg()));
                   } },
            // LOAD_FAST list, (TAIL | HEAD), (STORE | SET_VAL a)
            // ---> STORE_TAIL list a ; STORE_HEAD ; SET_VAL_TAIL ; SET_VAL_HEAD
            Rule { { LOAD_FAST, TAIL, STORE }, [](const Entities e) {
                      return IR::Entity(STORE_TAIL, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_FAST, TAIL, SET_VAL }, [](const Entities e) {
                      return IR::Entity(SET_VAL_TAIL, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_FAST, HEAD, STORE }, [](const Entities e) {
                      return IR::Entity(STORE_HEAD, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_FAST, HEAD, SET_VAL }, [](const Entities e) {
                      return IR::Entity(SET_VAL_HEAD, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_FAST_BY_INDEX, TAIL, STORE }, [](const Entities e) {
                      return IR::Entity(STORE_TAIL_BY_INDEX, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_FAST_BY_INDEX, TAIL, SET_VAL }, [](const Entities e) {
                      return IR::Entity(SET_VAL_TAIL_BY_INDEX, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_FAST_BY_INDEX, HEAD, STORE }, [](const Entities e) {
                      return IR::Entity(STORE_HEAD_BY_INDEX, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_FAST_BY_INDEX, HEAD, SET_VAL }, [](const Entities e) {
                      return IR::Entity(SET_VAL_HEAD_BY_INDEX, e[0].primaryArg(), e[2].primaryArg());
                  } },
            // (LOAD_CONST id | LOAD_FAST id), <comparison operator>, POP_JUMP_IF_(FALSE|TRUE)
            // ---> <OP>_(CONST|SYM)_JUMP_IF_(FALSE|TRUE)
            Rule { { LOAD_CONST, LT, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], LT_CONST_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
            Rule { { LOAD_CONST, LT, POP_JUMP_IF_TRUE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], LT_CONST_JUMP_IF_TRUE, e[0].primaryArg());
                  } },
            Rule { { LOAD_FAST, LT, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], LT_SYM_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
            Rule { { LOAD_CONST, GT, POP_JUMP_IF_TRUE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], GT_CONST_JUMP_IF_TRUE, e[0].primaryArg());
                  } },
            Rule { { LOAD_CONST, GT, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], GT_CONST_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
            Rule { { LOAD_FAST, GT, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], GT_SYM_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
            Rule { { LOAD_CONST, EQ, POP_JUMP_IF_TRUE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], EQ_CONST_JUMP_IF_TRUE, e[0].primaryArg());
                  } },
            Rule { { LOAD_FAST_BY_INDEX, EQ, POP_JUMP_IF_TRUE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], EQ_SYM_INDEX_JUMP_IF_TRUE, e[0].primaryArg());
                  } },
            Rule { { LOAD_CONST, NEQ, POP_JUMP_IF_TRUE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], NEQ_CONST_JUMP_IF_TRUE, e[0].primaryArg());
                  } },
            Rule { { LOAD_FAST, NEQ, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[2], NEQ_SYM_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
            // LOAD_FAST id, LOAD_FAST id2, AT
            // ---> AT_SYM_SYM id id2
            Rule { { LOAD_FAST, LOAD_FAST, AT }, AT_SYM_SYM },
            Rule { { LOAD_FAST_BY_INDEX, LOAD_FAST_BY_INDEX, AT }, AT_SYM_INDEX_SYM_INDEX },
            Rule { { LOAD_FAST_BY_INDEX, LOAD_CONST, AT }, AT_SYM_INDEX_CONST },
            // LOAD_FAST sym, TYPE, LOAD_CONST cst, EQ
            // ---> CHECK_TYPE_OF sym, cst
            // also works with LOAD_CONST cst, LOAD_FAST sym, TYPE, EQ, but args will be flipped
            Rule { { LOAD_FAST, TYPE, LOAD_CONST, EQ }, [](const Entities e) {
                      return IR::Entity(CHECK_TYPE_OF, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_CONST, LOAD_FAST, TYPE, EQ }, [](const Entities e) {
                      return IR::Entity(CHECK_TYPE_OF, e[1].primaryArg(), e[0].primaryArg());
                  } },
            Rule { { LOAD_FAST_BY_INDEX, TYPE, LOAD_CONST, EQ }, [](const Entities e) {
                      return IR::Entity(CHECK_TYPE_OF_BY_INDEX, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_CONST, LOAD_FAST_BY_INDEX, TYPE, EQ }, [](const Entities e) {
                      return IR::Entity(CHECK_TYPE_OF_BY_INDEX, e[1].primaryArg(), e[0].primaryArg());
                  } },
            // ---
            Rule { { LOAD_FAST_BY_INDEX, LEN, STORE }, [](const Entities e) {
                      return IR::Entity(STORE_LEN, e[0].primaryArg(), e[2].primaryArg());
                  } },
            Rule { { LOAD_FAST, LEN, LT, POP_JUMP_IF_FALSE }, [](const Entities e) {
                      return IR::Entity::GotoWithArg(e[3], LT_LEN_SYM_JUMP_IF_FALSE, e[0].primaryArg());
                  } },
        };

        const auto math_ops = { ADD, SUB, MUL, DIV };
        for (const auto& one : math_ops)
        {
            for (const auto& two : math_ops)
            {
                for (const auto& three : math_ops)
                    m_ruleset.emplace_back(Rule { { one, two, three }, fuseMathOps3 });
            }
        }

        for (const auto& one : math_ops)
        {
            for (const auto& two : math_ops)
                m_ruleset.emplace_back(Rule { { one, two }, fuseMathOps2 });
        }

        m_logger.debug("Loaded {} rules", m_ruleset.size());
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
                    std::span(
                        block.begin() + static_cast<IR::Block::difference_type>(i),
                        block.size() - i),
                    i);

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

    std::optional<EntityWithOffset> IROptimizer::replaceWithRules(const std::span<const IR::Entity> entities, const std::size_t position_in_block)
    {
        for (const auto& [expected, condition, createReplacement] : m_ruleset)
        {
            if (match(expected, entities) && condition(entities, position_in_block))
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

    bool IROptimizer::isSmallerNumberInlinable(const uint16_t id) const
    {
        if (std::cmp_less(id, m_values.size()) && m_values[id].type == ValTableElemType::Number)
        {
            const double val = std::get<double>(m_values[id].value) + IR::MaxValueForSmallNumber;
            return val >= 0.0 &&
                val < IR::MaxValueForDualArg &&
                static_cast<double>(static_cast<long>(val)) == val;
        }
        return false;
    }

    bool IROptimizer::isNumberEqualTo(const uint16_t id, const int number) const
    {
        if (std::cmp_less(id, m_values.size()) && m_values[id].type == ValTableElemType::Number)
        {
            const double val = std::get<double>(m_values[id].value);
            return static_cast<double>(static_cast<long>(val)) == val &&
                static_cast<int>(val) == number;
        }
        return false;
    }

    uint16_t IROptimizer::numberAsArg(const uint16_t id) const
    {
        return static_cast<uint16_t>(std::get<double>(m_values[id].value));
    }

    uint16_t IROptimizer::smallerNumberAsArg(const uint16_t id) const
    {
        return static_cast<uint16_t>(std::get<double>(m_values[id].value) + IR::MaxValueForSmallNumber);
    }
}
