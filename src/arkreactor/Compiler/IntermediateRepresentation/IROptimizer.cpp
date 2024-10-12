#include <Ark/Compiler/IntermediateRepresentation/IROptimizer.hpp>

namespace Ark::internal
{
    IROptimizer::IROptimizer(const unsigned debug) :
        m_logger("IROptimizer", debug)
    {}

    void IROptimizer::process(const std::vector<IR::Block>& pages, const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values)
    {
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
                const Instruction first = block[i].inst();
                const uint16_t arg_1 = block[i].primaryArg();

                if (i + 1 < end)
                {
                    const Instruction second = block[i + 1].inst();
                    const uint16_t arg_2 = block[i + 1].primaryArg();

                    // LOAD_CONST x
                    // LOAD_CONST y
                    // ---> LOAD_CONST_LOAD_CONST x y
                    if (first == LOAD_CONST && second == LOAD_CONST)
                    {
                        current_block.emplace_back(LOAD_CONST_LOAD_CONST, arg_1, arg_2);
                        i += 2;
                    }
                    // LOAD_CONST x
                    // STORE / SET_VAL a
                    // ---> LOAD_CONST_STORE x a ; LOAD_CONST_SET_VAL x a
                    else if (first == LOAD_CONST && second == STORE)
                    {
                        current_block.emplace_back(LOAD_CONST_STORE, arg_1, arg_2);
                        i += 2;
                    }
                    else if (first == LOAD_CONST && second == SET_VAL)
                    {
                        current_block.emplace_back(LOAD_CONST_SET_VAL, arg_1, arg_2);
                        i += 2;
                    }
                    // LOAD_SYMBOL a
                    // STORE / SET_VAL b
                    // ---> STORE_FROM a b ; SET_VAL_FROM a b
                    else if (first == LOAD_SYMBOL && second == STORE)
                    {
                        current_block.emplace_back(STORE_FROM, arg_1, arg_2);
                        i += 2;
                    }
                    else if (first == LOAD_SYMBOL && second == SET_VAL)
                    {
                        current_block.emplace_back(SET_VAL_FROM, arg_1, arg_2);
                        i += 2;
                    }
                    else if (i + 2 < end)
                    {
                        const Instruction third = block[i + 2].inst();
                        const uint16_t arg_3 = block[i + 2].primaryArg();

                        // LOAD_SYMBOL a
                        // LOAD_CONST n (1)
                        // ADD / SUB
                        // ---> INCREMENT / DECREMENT a
                        if (third == ADD && first == LOAD_CONST && second == LOAD_SYMBOL && m_values[arg_1].type == ValTableElemType::Number && std::get<double>(m_values[arg_1].value) == 1)
                        {
                            current_block.emplace_back(INCREMENT, arg_2);
                            i += 3;
                        }
                        else if (third == ADD && first == LOAD_SYMBOL && second == LOAD_CONST && m_values[arg_2].type == ValTableElemType::Number && std::get<double>(m_values[arg_2].value) == 1)
                        {
                            current_block.emplace_back(INCREMENT, arg_1);
                            i += 3;
                        }
                        else if (third == SUB && first == LOAD_SYMBOL && second == LOAD_CONST && m_values[arg_2].type == ValTableElemType::Number && std::get<double>(m_values[arg_2].value) == 1)
                        {
                            current_block.emplace_back(DECREMENT, arg_1);
                            i += 3;
                        }
                        // LOAD_SYMBOL list
                        // TAIL / HEAD
                        // STORE / SET_VAL a
                        // ---> STORE_TAIL list a ; STORE_HEAD ; SET_VAL_TAIL ; SET_VAL_HEAD
                        else if (first == LOAD_SYMBOL && second == TAIL && third == STORE)
                        {
                            current_block.emplace_back(STORE_TAIL, arg_1, arg_3);
                            i += 3;
                        }
                        else if (first == LOAD_SYMBOL && second == TAIL && third == SET_VAL)
                        {
                            current_block.emplace_back(SET_VAL_TAIL, arg_1, arg_3);
                            i += 3;
                        }
                        else if (first == LOAD_SYMBOL && second == HEAD && third == STORE)
                        {
                            current_block.emplace_back(STORE_HEAD, arg_1, arg_3);
                            i += 3;
                        }
                        else if (first == LOAD_SYMBOL && second == HEAD && third == SET_VAL)
                        {
                            current_block.emplace_back(SET_VAL_HEAD, arg_1, arg_3);
                            i += 3;
                        }
                        else
                        {
                            current_block.emplace_back(block[i]);
                            ++i;
                        }
                    }
                    else
                    {
                        current_block.emplace_back(block[i]);
                        ++i;
                    }
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
}
