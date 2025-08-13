/**
 * @file Dict.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Define how dictionaries are handled
 * @date 2025-08-03
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef ARK_VM_VALUE_DICT_HPP
#define ARK_VM_VALUE_DICT_HPP

#include <Ark/VM/Value.hpp>
#include <Ark/Utils/Platform.hpp>

#include <ankerl/unordered_dense.h>

#include <vector>

namespace Ark
{
    class VM;
}

namespace Ark::internal
{
    class ARK_API Dict
    {
    public:
        Dict() = default;

        /**
         * @brief Assign a key to a value inside the dict
         *
         * @param key
         * @param value
         */
        void set(const Value& key, const Value& value);

        /**
         * @brief Try to get a value from a given key. If no value is found, return Nil
         *
         * @param key
         * @return const Value&
         */
        const Value& get(const Value& key);

        /**
         * @brief Check that a key exists
         *
         * @param key
         * @return true if the dict has the key
         * @return false otherwise
         */
        [[nodiscard]] bool contains(const Value& key) const;

        /**
         * @brief Remove an entry from the dict via its key
         *
         * @param key
         */
        void remove(const Value& key);

        /**
         * @brief Get a list of the dict keys
         *
         * @return std::vector<Value>
         */
        std::vector<Value> keys();

        /**
         * @brief Compute the number of (key, value) pairs in the dict
         *
         * @return std::size_t
         */
        [[nodiscard]] std::size_t size() const;

        /**
         * @brief Convert the dictionary to a string for pretty printing
         *
         * @param vm
         * @return std::string
         */
        std::string toString(VM& vm) const;

        friend bool operator==(const Dict&, const Dict&) noexcept;

    private:
        ankerl::unordered_dense::map<Value, Value> m_dict;
    };

    inline bool operator==(const Dict& A, const Dict& B) noexcept
    {
        return A.m_dict == B.m_dict;
    }
}

#endif  // ARK_VM_VALUE_DICT_HPP
