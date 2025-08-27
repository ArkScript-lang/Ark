#ifndef ARK_COMPILER_AST_IMPORT_HPP
#define ARK_COMPILER_AST_IMPORT_HPP

#include <vector>
#include <string>
#include <numeric>

#include <Ark/Utils/Platform.hpp>

namespace Ark::internal
{
    struct ARK_API Import
    {
        std::size_t line, col;  ///< Position in the source file

        /**
         * @brief The filename without the extension
         * @details Example: `(import foo.bar)` => `bar`
         *      `(import foo.bar.egg:*)` => `egg`
         *      `(import foo :a :b :c)` => `foo`
         *
         */
        std::string prefix;

        /**
         * @brief Package with all the segments
         * @details Example: `(import foo.bar)` => `{foo, bar}`
         *      `(import foo.bar.egg:*)` => `{foo, bar, egg}`
         *      `(import foo :a :b :c)` => `{foo}`
         */
        std::vector<std::string> package;

        /**
         * @brief Import with prefix (import package)
         *
         */
        bool with_prefix = true;

        /**
         * @brief Import as glob (import package:*)
         */
        bool is_glob = false;

        /**
         * @brief List of symbols to import, can be empty if none provided. (import package :a :b)
         *
         */
        std::vector<std::string> symbols;

        /**
         *
         * @return a package string, eg a.b.c
         */
        [[nodiscard]] std::string toPackageString() const
        {
            return std::accumulate(
                package.begin() + 1,
                package.end(),
                package.front(),
                [](const std::string& left, const std::string& right) {
                    return left + "." + right;
                });
        }

        /**
         *
         * @return a package as a path, eg a/b/c
         */
        [[nodiscard]] std::string packageToPath() const
        {
            return std::accumulate(
                std::next(package.begin()),
                package.end(),
                package[0],
                [](const std::string& a, const std::string& b) {
                    return a + "/" + b;
                });
        }

        /**
         * @brief Check if we should import everything with a prefix, given a `(import foo.bar.egg)`
         *
         * @return true
         * @return false
         */
        [[nodiscard]] bool isBasic() const
        {
            return with_prefix && symbols.empty();
        }
    };
}

#endif
