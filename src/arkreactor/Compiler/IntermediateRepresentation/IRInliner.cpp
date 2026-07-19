#include <Ark/Compiler/IntermediateRepresentation/IRInliner.hpp>

#include <algorithm>
#include <cassert>

namespace Ark::internal
{
    IRInliner::IRInliner(const unsigned debug) :
        Pass("IRInliner", debug),
        m_current_label(0)
    {}

    void IRInliner::process(const std::vector<IR::Block>& pages, const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values, const IR::label_t last_label)
    {
        m_logger.traceStart("process");
        m_symbols = symbols;
        m_values = values;
        m_current_label = last_label + 1;

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
            for (std::size_t i = 0, end = block.data.size(); i < end; ++i)
            {
                const auto& entity = block.data[i];

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

                    // retrieve the return label of the call instruction, to know which PUSH_RETURN_ADDRESS instruction we'll have to remove
                    if (i + 1 < end)
                    {
                        assert(block.data[i + 1].kind() == IR::Kind::Label && "Expected a label right after the CALL instruction! The AST lowerer messed up somewhere");

                        const IR::label_t return_label = block.data[i + 1].label();
                        const std::size_t removed = std::erase_if(new_block.data, [return_label](const IR::Entity& e) -> bool {
                            return e.kind() == IR::Kind::Goto && e.inst() == PUSH_RETURN_ADDRESS && e.label() == return_label;
                        });

                        if (removed == 0)
                            throw std::runtime_error(fmt::format("No PUSH_RETURN_ADDRESS L{} instruction removed, even though one was expected", return_label));
                    }

                    inlineBlock(inlinee, new_block);
                }
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
            candidate.metadata.is_mutating_args ||
            candidate.metadata.name.value_or(std::string(IR::AnonymousBlockName)) == IR::AnonymousBlockName ||
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

    std::optional<IR::Entity> IRInliner::isBuiltinProxy(const IR::Block& block)
    {
        /*
         Expected instructions to be a builtin proxy:
            STORE...
            PUSH_RETURN_ADDRESS
            LOAD_FAST_BY_INDEX...
            CALL_BUILTIN
            <label>
            RET
         */
        if (block.data.size() < 6 || (block.data.size() - 4) % 2 != 0)
            return std::nullopt;

        Instruction expected = STORE;
        std::size_t store_count = 0;
        std::size_t load_count = 0;
        IR::label_t label = 0;
        std::optional<IR::Entity> call_builtin;

        for (const auto& entity : block.data)
        {
            const Instruction inst = entity.inst();
            const bool is_label = entity.kind() == IR::Kind::Label;

            if (expected != inst)
            {
                if (expected == STORE)
                    expected = PUSH_RETURN_ADDRESS;
                else if (expected == PUSH_RETURN_ADDRESS)
                    expected = LOAD_FAST_BY_INDEX;
                else if (expected == LOAD_FAST_BY_INDEX)
                    expected = CALL_BUILTIN;
                else if (expected == CALL_BUILTIN)
                    expected = NOP;
                else if (expected == NOP)
                    expected = RET;
            }

            if (expected == inst)
            {
                if (expected == STORE)
                    ++store_count;
                else if (expected == LOAD_FAST_BY_INDEX)
                    ++load_count;
                else if (expected == CALL_BUILTIN)
                {
                    call_builtin = entity;
                    if (entity.secondaryArg() != store_count)
                        return std::nullopt;
                }
                else if (expected == PUSH_RETURN_ADDRESS)
                    label = entity.label();
            }
            else if (is_label && label != entity.label())
                return std::nullopt;
        }

        if (store_count == load_count)
            return call_builtin;
        return std::nullopt;
    }

    void IRInliner::inlineBlock(const IR::Block& inlinee, IR::Block& destination)
    {
        if (destination.metadata.addr == 0)
            m_logger.info("Inlining call to '{}' ({}) inside global scope", inlinee.debugName(), inlinee.metadataRepr());
        else
            m_logger.info(
                "Inlining call to '{}' ({} from '{}') inside '{}' @ {}, from '{}'",
                inlinee.debugName(),
                inlinee.metadataRepr(),
                inlinee.data.front().filename(),
                destination.debugName(),
                destination.metadata.addr,
                destination.data.front().filename());

        if (auto inst = isBuiltinProxy(inlinee); inst.has_value())
        {
            m_logger.info("  -> builtin proxy with args ({}, {})", inst->primaryArg(), inst->secondaryArg());
            destination.data.emplace_back(CALL_BUILTIN_WITHOUT_RETURN_ADDRESS, inst->primaryArg(), inst->secondaryArg());
            return;
        }

        // TODO: do a better inlining job
        //       this can currently fuck up symbol indices (LOAD_FAST_BY_ID), overwrite variables from the parents, and maybe more
        //       -> fixed if we put a scope around
        // TODO: decide if we want to keep create_scope, (inlinee), pop_scope
        destination.data.emplace_back(CREATE_SCOPE);

        // We need to create new, unique labels for the inlined code.
        // When we meet a label, we'll register it, and replace it with a new label
        // in the inlined code. That way, if we find it again later in the code,
        // we can use the correct value.
        std::unordered_map<IR::label_t, IR::label_t> old_to_new_label;

        for (const IR::Entity& entity : inlinee.data)
        {
            if (entity.inst() == RET)
                break;

            if (entity.hasLabel())
            {
                IR::Entity labelled_entity = entity;
                if (auto it = old_to_new_label.find(entity.label()); it != old_to_new_label.end())
                    labelled_entity.replaceLabel(it->second);
                else
                {
                    labelled_entity.replaceLabel(m_current_label);
                    old_to_new_label[entity.label()] = m_current_label++;
                }

                destination.data.emplace_back(labelled_entity);
            }
            else if (entity.inst() == LOAD_FAST_BY_INDEX)
                destination.data.emplace_back(LOAD_FAST, entity.originalSymbolId().value());
            else if (entity.inst() == CALL_SYMBOL_BY_INDEX)
                destination.data.emplace_back(CALL_SYMBOL, entity.originalSymbolId().value(), entity.secondaryArg());
            else
                destination.data.emplace_back(entity);
        }

        destination.data.emplace_back(POP_SCOPE, 1);
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
