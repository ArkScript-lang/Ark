/**
 * @file ScopeView.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief The virtual machine scope system
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2026
 *
 */

#ifndef ARK_VM_SCOPE_HPP
#define ARK_VM_SCOPE_HPP

#include <cinttypes>

#include <Ark/Constants.hpp>
#include <Ark/Utils/Platform.hpp>
#include <Ark/VM/Value/Value.hpp>

namespace Ark::internal
{
    /**
     * @brief A class to handle the VM scope more efficiently
     *
     */
    class ARK_API ScopeView
    {
    public:
        using pair_t = std::pair<uint16_t, Value>;

        /**
         * @brief Deleted constructor to avoid creating ScopeViews pointing to nothing. Helps catch bugs at compile time
         */
        ScopeView() = delete;

        /**
         * @brief Create a new ScopeView
         *
         * @param storage pointer to the shared scope storage
         * @param start first free starting position
         */
        ARK_ALWAYS_INLINE ScopeView(pair_t* storage, const std::size_t start) noexcept :
            m_storage(storage), m_start(start), m_size(0), m_min_id(MaxValue16Bits), m_max_id(0)
        {}

        /**
         * @brief Put a value in the scope
         *
         * @param id The symbol id of the variable
         * @param val The value linked to the symbol
         */
        ARK_ALWAYS_INLINE void pushBack(uint16_t id, Value&& val) noexcept
        {
            if (id < m_min_id)
                m_min_id = id;
            if (id > m_max_id)
                m_max_id = id;

            m_storage[m_start + m_size] = std::make_pair(id, std::move(val));
            ++m_size;
        }

        /**
         * @brief Put a value in the scope
         *
         * @param id The symbol id of the variable
         * @param val The value linked to the symbol
         */
        ARK_ALWAYS_INLINE void pushBack(uint16_t id, const Value& val) noexcept
        {
            if (id < m_min_id)
                m_min_id = id;
            if (id > m_max_id)
                m_max_id = id;

            m_storage[m_start + m_size] = std::make_pair(id, val);
            ++m_size;
        }

        /**
         * @brief Insert one or more pairs at the beginning of the scope
         * @details This can ONLY be called on the last known scope, otherwise it will override the data of the next scope!
         *
         * @param values
         */
        void insertFront(const std::vector<pair_t>& values) noexcept;

        /**
         * @brief Check if the scope maybe holds a specific symbol in memory
         *
         * @param id The id of the symbol
         * @return true On success
         * @return false Otherwise
         */
        [[nodiscard]] ARK_ALWAYS_INLINE bool maybeHas(const uint16_t id) const noexcept
        {
            return m_min_id <= id && id <= m_max_id;
        }

        /**
         * @brief Get a value from its symbol id
         *
         * @param id_to_look_for
         * @return Value* Returns nullptr if the value can not be found
         */
        [[nodiscard]] ARK_ALWAYS_INLINE Value* operator[](const uint16_t id_to_look_for) noexcept
        {
            if (!maybeHas(id_to_look_for))
                return nullptr;

            for (std::size_t i = m_start; i < m_start + m_size; ++i)
            {
                auto& [id, value] = m_storage[i];
                if (id == id_to_look_for)
                    return &value;
            }
            return nullptr;
        }

        /**
         * @brief Get a value from its symbol id
         *
         * @param id_to_look_for
         * @return const Value* Returns nullptr if the value can not be found
         */
        [[nodiscard]] ARK_ALWAYS_INLINE const Value* operator[](const uint16_t id_to_look_for) const noexcept
        {
            if (!maybeHas(id_to_look_for))
                return nullptr;

            for (std::size_t i = m_start; i < m_start + m_size; ++i)
            {
                auto& [id, value] = m_storage[i];
                if (id == id_to_look_for)
                    return &value;
            }
            return nullptr;
        }

        /**
         * @brief Get the id of a variable based on its value ; used for debug only
         *
         * @param val
         * @return uint16_t
         */
        [[nodiscard]] uint16_t idFromValue(const Value& val) const noexcept;

        /**
         * @brief Return the element at index in scope
         *
         * @return const pair_t&
         */
        [[nodiscard]] ARK_ALWAYS_INLINE const pair_t& atPos(const std::size_t i) const noexcept
        {
            return m_storage[m_start + i];
        }

        /**
         * @brief Return the element at index, starting from the end
         *
         * @return const pair_t&
         */
        [[nodiscard]] ARK_ALWAYS_INLINE pair_t& atPosReverse(const std::size_t i) const noexcept
        {
            return m_storage[m_start + m_size - 1 - i];
        }

        /**
         * @brief Reset size, min and max id for the scope, to signify it's empty
         */
        ARK_ALWAYS_INLINE void reset() noexcept
        {
            m_size = 0;
            m_min_id = MaxValue16Bits;
            m_max_id = 0;
        }

        /**
         * @brief Return the size of the scope
         *
         * @return const std::size_t
         */
        [[nodiscard]] ARK_ALWAYS_INLINE std::size_t size() const noexcept
        {
            return m_size;
        }

        /**
         * @brief Compute the position of the first free slot in the shared storage, after this scope
         *
         * @return std::size_t
         */
        [[nodiscard]] ARK_ALWAYS_INLINE std::size_t storageEnd() const noexcept
        {
            return m_start + m_size;
        }

        friend ARK_API bool operator==(const ScopeView& A, const ScopeView& B) noexcept;

        friend class Ark::VM;

    private:
        pair_t* m_storage;
        std::size_t m_start;
        std::size_t m_size;
        uint16_t m_min_id;  ///< Minimum stored ID, used for a basic bloom filter
        uint16_t m_max_id;  ///< Maximum stored ID, used for a basic bloom filter
    };
}

#endif
