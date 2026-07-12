#include <Ark/Compiler/IntermediateRepresentation/IRInliner.hpp>

#include <algorithm>
#include <cassert>

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

        extractPagesMetadata(pages);

        // TODO: we'll need to move some page index if a page is removed!
        // TODO: some pages could be removed if they are inlined everywhere!
        // TODO: should we start from the end to inline as much as we can? or do we want to inline the user code first, then the stdlib?
        for (const auto& block : pages)
        {
            IR::Block new_block {
                .metadata = {
                    .name = block.metadata.name,
                    .argument_count = block.metadata.argument_count,
                    .addr = block.metadata.addr,
                    .is_closure = block.metadata.is_closure,
                    .is_recursive = block.metadata.is_recursive,
                    .is_simple = block.metadata.is_simple },
                .data = {}
            };

            // We only have to deal with CALL_SYMBOL, CALL_SYMBOL_BY_INDEX, which deal with symbols,
            // and CALL which can deal with constant ids (eg `((fun (a) (print a)) 5)`)
            for (const auto& entity : block.data)
            {
                std::optional<uint16_t> maybe_id;
                CallKind kind = CallKind::Symbol;

                if (entity.inst() == CALL_SYMBOL || entity.inst() == CALL_SYMBOL_BY_INDEX)
                    maybe_id = entity.originalSymbolId();
                else if (entity.inst() == CALL)
                {
                    // TODO: find the function being called, check if it's a constant, then if we know it
                    maybe_id = {};
                    kind = CallKind::Constant;
                }

                if (const auto maybe_block = blockToInlineInCall(kind, pages, maybe_id, block); maybe_block.has_value())
                {
                    const IR::Block& inlinee = pages[maybe_block->addr];

                    if (new_block.data[new_block.data.size() - 1 - inlinee.metadata.argument_count].inst() == PUSH_RETURN_ADDRESS)
                        new_block.data.erase(new_block.data.end() - static_cast<long>(1 + inlinee.metadata.argument_count));

//                    new_block.data.emplace_back(CREATE_SCOPE);
                    inlineBlock(inlinee, new_block);
//                    new_block.data.emplace_back(POP_SCOPE);
                }
                // TODO: find a better way to not fuck up/repair the indices
                else if (entity.inst() == LOAD_FAST_BY_INDEX)
                    new_block.data.emplace_back(LOAD_FAST, entity.originalSymbolId().value());
                else if (entity.inst() == CALL_SYMBOL_BY_INDEX)
                    new_block.data.emplace_back(CALL_SYMBOL, entity.originalSymbolId().value(), entity.secondaryArg());
                else
                    new_block.data.emplace_back(entity);
            }

            m_ir.emplace_back(new_block);
        }

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

    std::optional<BlockInfo> IRInliner::blockToInlineInCall(const CallKind kind, const std::vector<IR::Block>& pages, const std::optional<uint16_t> maybe_id, const IR::Block& current) const noexcept
    {
        if (!maybe_id.has_value())
            return std::nullopt;

        const uint16_t id = maybe_id.value();
        std::optional<BlockInfo> maybe_block = findBlockBy(kind, id);
        if (!maybe_block.has_value())
            return std::nullopt;

        const std::size_t block_addr = maybe_block->addr;
        if (canBeInlined(pages[block_addr], current))
            return maybe_block;
        return std::nullopt;
    }

    void IRInliner::inlineBlock(const IR::Block& inlinee, IR::Block& destination)
    {
        m_logger.debug("Inlining call to {} inside {} @ {}", inlinee.debugName(), destination.debugName(), destination.metadata.addr);

        // TODO: do a better inlining job
        // this can currently fuck up symbol indices (LOAD_FAST_BY_ID), overwrite variables from the parents, and maybe more
        for (const IR::Entity& entity : inlinee.data)
        {
            if (entity.inst() == RET)
                break;

            if (entity.inst() == LOAD_FAST_BY_INDEX)
                destination.data.emplace_back(LOAD_FAST, entity.originalSymbolId().value());
            else if (entity.inst() == CALL_SYMBOL_BY_INDEX)
                destination.data.emplace_back(CALL_SYMBOL, entity.originalSymbolId().value(), entity.secondaryArg());
            else
                destination.data.emplace_back(entity);
        }
    }

    void IRInliner::extractPagesMetadata(const std::vector<IR::Block>& pages)
    {
        for (std::size_t i = 0, end = pages.size(); i < end; ++i)
        {
            const std::string& name = pages[i].debugName();
            if (name != IR::AnonymousBlockName)
            {
                const auto it_val = std::ranges::find_if(m_values, [i](const ValTableElem& elem) -> bool {
                    return elem.type == ValTableElemType::PageAddr && std::get<std::size_t>(elem.value) == i;
                });
                assert(it_val != m_values.end() && "Could not find a constant referencing the current page!");

                const auto it_sym = std::ranges::find_if(m_symbols, [&name](const std::string& sym) -> bool {
                    return name == sym;
                });

                m_funcs.emplace_back(BlockInfo {
                    .constant_id = static_cast<long>(std::distance(m_values.begin(), it_val)),
                    .addr = i,
                    .name = pages[i].debugName(),
                    .symbol_id = it_sym == m_symbols.end()
                        ? std::nullopt
                        : std::make_optional(std::distance(m_symbols.begin(), it_sym)) });
            }
        }
    }

    std::optional<BlockInfo> IRInliner::findBlockBy(const CallKind kind, const uint16_t id) const noexcept
    {
        const auto it = std::ranges::find_if(
            m_funcs,
            [id, kind](const BlockInfo& info) -> bool {
                switch (kind)
                {
                    case CallKind::Symbol:
                        return info.symbol_id.has_value() && std::cmp_equal(info.symbol_id.value(), id);

                    case CallKind::Constant:
                        return std::cmp_equal(info.constant_id, id);
                }
                return false;
            });

        if (it != m_funcs.end())
            return *it;
        return std::nullopt;
    }
}
