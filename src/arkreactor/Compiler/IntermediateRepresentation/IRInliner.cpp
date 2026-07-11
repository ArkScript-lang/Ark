#include <Ark/Compiler/IntermediateRepresentation/IRInliner.hpp>

namespace Ark::internal
{
    IRInliner::IRInliner(const unsigned debug) :
        Pass("IRInliner", debug)
    {}

    void IRInliner::process(const std::vector<IR::Block>& pages, const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values)
    {
        m_logger.traceStart("process");
        m_symbols = symbols;
        m_values = values;

        // TODO
        m_ir = pages;

        m_logger.traceEnd();
    }

    const std::vector<IR::Block>& IRInliner::intermediateRepresentation() const noexcept
    {
        return m_ir;
    }
}
