/**
 * @file IRInliner.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Try to inline IR blocks
 * @date 2026-07-11
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef ARK_COMPILER_INTERMEDIATEREPRESENTATION_IRINLINER_HPP
#define ARK_COMPILER_INTERMEDIATEREPRESENTATION_IRINLINER_HPP

#include <Ark/Utils/Platform.hpp>
#include <Ark/Compiler/Pass.hpp>
#include <Ark/Compiler/ValTableElem.hpp>
#include <Ark/Compiler/IntermediateRepresentation/Entity.hpp>

namespace Ark::internal
{
    class ARK_API IRInliner final : public Pass
    {
    public:
        explicit IRInliner(unsigned debug);

        void process(const std::vector<IR::Block>& pages, const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values);

        const std::vector<IR::Block>& intermediateRepresentation() const noexcept;

    private:
        std::vector<IR::Block> m_ir;
        std::vector<std::string> m_symbols;
        std::vector<ValTableElem> m_values;
    };
}

#endif  // ARK_COMPILER_INTERMEDIATEREPRESENTATION_IRINLINER_HPP
