#ifndef ark_vm_usertype
#define ark_vm_usertype

#include <iostream>
#include <vector>
#include <utility>
#include <typeindex>

namespace Ark
{
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
            std::ostream& (*ostream_func) (std::ostream&, const UserType&) = nullptr;
            void          (*deleter) (void*)                               = nullptr;
        };

        /**
         * @brief Construct a new User Type object
         * 
         * @tparam T the type of the pointer
         * @param data a pointer to the data to store in the object
         */
        template <typename T>
        explicit UserType(T* data=nullptr) :
            m_data(static_cast<void*>(data)),
            m_funcs(nullptr),
            m_type_id(typeid(T).hash_code() & static_cast<uint16_t>(~0))
        {}

        /**
         * @brief Destroy the User Type object
         * @details Called by the VM when `(del obj)` is found or when the object goes
         *          out of scope.
         * 
         */
         void del()
         {
             // call a custom deleter on the data held by the usertype
             if (m_funcs != nullptr && m_funcs->deleter != nullptr)
                m_funcs->deleter(m_data);
         }

        /**
         * @brief Set the control functions structure
         * 
         * @param block A pointer to an instance of this block
         */
        inline void setControlFuncs(ControlFuncs* block);

        /**
         * @brief Get the pointer to the object
         * 
         * @return void* 
         */
        inline void* data() const;

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
        bool is() const
        {
            return (typeid(T).hash_code() & static_cast<uint16_t>(~0)) == m_type_id;
        }

        /**
         * @brief Return the underlying object as a given type
         * 
         * @tparam T the type in which the underlying data pointer should be converted to
         * @return T& 
         */
        template <typename T>
        T& as()
        {
            return *static_cast<T*>(m_data);
        }

        friend inline bool operator==(const UserType& A, const UserType& B);
        friend inline bool operator<(const UserType& A, const UserType& B);
        friend inline std::ostream& operator<<(std::ostream& os, const UserType& A);

    private:
        uint16_t m_type_id;
        void* m_data;
        ControlFuncs* m_funcs;
    };

    #include "inline/UserType.inl"
}

#endif