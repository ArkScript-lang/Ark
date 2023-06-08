#ifndef COMPILER_AST_IMPORT_HPP
#define COMPILER_AST_IMPORT_HPP

#include <vector>
#include <string>

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
        std::string package;

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

        /**
         * @brief Check if we should import everything, given something like `(import foo.bar.egg:*)`
         * 
         * @return true if all symbols of the file should be imported in the importer scope
         * @return false otherwise
         */
        inline bool isGlob() const
        {
            return !with_prefix && symbols.empty();
        }

        /**
         * @brief Check if we should import everything with a prefix, given a `(import foo.bar.egg)`
         * 
         * @return true 
         * @return false 
         */
        inline bool isBasic() const
        {
            return with_prefix && symbols.empty();
        }
    };
}

#endif
