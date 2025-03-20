/**
 * @file LocalsLocator.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Track locals at compile
 * @version 0.1
 * @date 2025-03-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef ARK_COMPILER_LOWERER_LOCALSLOCATOR_HPP
#define ARK_COMPILER_LOWERER_LOCALSLOCATOR_HPP

#include <vector>
#include <string>
#include <optional>

namespace Ark::internal
{
    class LocalsLocator
    {
    public:
        enum class ScopeType
        {
            Default,
            Function,
            Closure
        };

        LocalsLocator();

        void addLocal(const std::string& name);
        std::optional<std::size_t> lookupLastScopeByName(const std::string& name);

        void createScope(ScopeType type = ScopeType::Default);
        void deleteScope();

    private:
        struct Scope
        {
            std::vector<std::string> data;
            ScopeType type;
        };

        std::vector<Scope> m_scopes;
    };
}

#endif  // ARK_COMPILER_LOWERER_LOCALSLOCATOR_HPP
