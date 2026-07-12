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

    bool IRInliner::canBeInlined(const IR::Block& candidate, const IR::Block& source) noexcept
    {
        const std::size_t candidate_inst_count = candidate.instructionCount(),
                          source_inst_count = source.instructionCount();

        if (candidate.metadata.is_closure ||
            candidate.metadata.is_recursive ||
            candidate.metadata.name.value_or(IR::AnonymousBlockName) == IR::AnonymousBlockName ||
            std::cmp_greater_equal(candidate_inst_count + source_inst_count, MaxValue16Bits))
            return false;
        // TODO: create a proper constant and make this less arbitrary?
        return candidate.metadata.is_simple && candidate_inst_count < 24;
    }
}
