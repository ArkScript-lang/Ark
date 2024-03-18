#ifndef COMPILER_AST_IMPORT_HPP
#define COMPILER_AST_IMPORT_HPP

#include <vector>
#include <string>
#include <numeric>

#include <Ark/Platform.hpp>

namespace Ark::internal
{
    struct ARK_API Import
    {
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
         * @brief Import with prefix (the package) or not
         *
         */
        bool with_prefix = true;

        /**
         * @brief List of symbols to import, can be empty if none provided
         *
         */
        std::vector<std::string> symbols;

        [[nodiscard]] inline std::string toPackageString() const
        {
            return std::accumulate(package.begin() + 1, package.end(), package.front(), [](const std::string& left, const std::string& right) {
                return left + "." + right;
            });
        }

        [[nodiscard]] inline std::string packageToPath() const
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
         * @brief Check if we should import everything, given something like `(import foo.bar.egg:*)`
         *
         * @return true if all symbols of the file should be imported in the importer scope
         * @return false otherwise
         */
        [[nodiscard]] inline bool isGlob() const
        {
            return !with_prefix && symbols.empty();
        }

        /**
         * @brief Check if we should import everything with a prefix, given a `(import foo.bar.egg)`
         *
         * @return true
         * @return false
         */
        [[nodiscard]] inline bool isBasic() const
        {
            return with_prefix && symbols.empty();
        }
    };
}

#endif
