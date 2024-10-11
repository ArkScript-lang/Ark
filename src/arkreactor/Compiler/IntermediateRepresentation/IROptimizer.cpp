#include <Ark/Compiler/IntermediateRepresentation/IROptimizer.hpp>

namespace Ark::internal
{
    IROptimizer::IROptimizer(const unsigned debug) :
        m_logger("IROptimizer", debug)
    {}

    void IROptimizer::process(const std::vector<IR::Block>& pages, const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values)
    {
        m_ir = pages;
        m_symbols = symbols;
        m_values = values;
    }

    const std::vector<IR::Block>& IROptimizer::intermediateRepresentation() const noexcept
    {
        return m_ir;
    }
}
