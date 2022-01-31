/**
 * @file UserType.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Subtype of the value, capable of handling any C++ type
 * @version 0.3
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */

#ifndef VM_VALUE_USERTYPE_HPP
#define VM_VALUE_USERTYPE_HPP

#include <iostream>
#include <vector>
#include <utility>
#include <memory>

#include <Ark/Platform.hpp>

namespace Ark
{
    namespace internal
    {
        struct type_uid_impl
        {
            static inline uint16_t id = 0;
            static uint16_t next()
            {
                return id++;
            }
        };

        template <typename T>
        struct type_uid
        {
            static inline const uint16_t value = type_uid_impl::next();
        };
    }

    /**
     * @brief A class to be use C++ objects in ArkScript
     * 
     * A pointer to the value you want to store
     * must be sent, thus the value must not be destroyed while the UserType lives,
     * otherwise it would result in an UB when trying to use the object
    */
    class UserType
    {
    public:
        /**
         * @brief A structure holding a bunch of pointers to different useful functions related to this usertype
         * 
         */
        struct ControlFuncs
        {
            std::ostream& (*ostream_func)(std::ostream&, const UserType&) = nullptr;
            void (*deleter)(void*) = nullptr;
        };

        /**
         * @brief Construct a new User Type object
         * 
         * @tparam T the type of the pointer
         * @param data a pointer to the data to store in the object
         */
        template <typename T>
        explicit UserType(T* data = nullptr, ControlFuncs* block = nullptr) noexcept :
            m_type_id(internal::type_uid<T>::value),
            m_data(reinterpret_cast<void*>(data)),
            m_funcs(block)
        {}

        /**
         * @brief Free memory through the control functions block
         * 
         */
        void del();

        /**
         * @brief Get the pointer to the object
         * 
         * @return void* 
         */
        void* data() const noexcept
        {
            return m_data;
        }

        /**
         * @brief Check if the object held is of a given type
         * @details Usage example:
         * @code
         * MyType object;
         * UserType a(&object);
         * if (a.is<MyType>())
         *     // then ...
         * else
         *     // otherwise...
         * @endcode
         * 
         * @tparam T the type to use for the test
         * @return true 
         * @return false 
         */
        template <typename T>
        bool is() const noexcept
        {
            return internal::type_uid<T>::value == m_type_id;
        }

        /**
         * @brief Return the underlying object as a given type
         * 
         * @tparam T the type in which the underlying data pointer should be converted to
         * @return T& 
         */
        template <typename T>
        T& as() noexcept
        {
            return *reinterpret_cast<T*>(m_data);
        }

        template <typename T>
        const T& as() const noexcept
        {
            return *reinterpret_cast<T*>(m_data);
        }

        friend ARK_API bool operator==(const UserType& A, const UserType& B) noexcept;
        friend ARK_API bool operator<(const UserType& A, const UserType& B) noexcept;
        friend ARK_API std::ostream& operator<<(std::ostream& os, const UserType& A) noexcept;

    private:
        uint16_t m_type_id;
        void* m_data;
        ControlFuncs* m_funcs;
    };

}

#endif
