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
        using FuncStream_t = std::ostream& (*) (std::ostream& os, const UserType& A);

        UserType() :
            m_data(nullptr), m_ostream_func(nullptr), m_type_id(std::type_index(typeid(void)))
        {}

        /**
         * @brief Construct a new User Type object
         * 
         * @tparam T the type of the pointer
         * @param data a pointer to the data to store in the object
         */
        template <typename T>
        explicit UserType(T* data=nullptr) :
            m_data(static_cast<void*>(data)),
            m_ostream_func(nullptr),
            m_type_id(std::type_index(typeid(T)))
        {}

        /**
         * @brief Set the ostream function
         * 
         * @param f the function
         */
        inline void setOStream(FuncStream_t&& f);

        /**
         * @brief Get the type id of the value held by the object
         * 
         * @return const std::type_index 
         */
        inline const std::type_index type_id() const;

        /**
         * @brief Get the pointer to the object
         * 
         * @return void* 
         */
        inline void* data() const;

        /**
         * @brief User implemented `not` operator
         * 
         * @return true 
         * @return false 
         */
        inline bool not_() const;

        friend inline bool operator==(const UserType& A, const UserType& B);
        friend inline bool operator<(const UserType& A, const UserType& B);
        friend inline std::ostream& operator<<(std::ostream& os, const UserType& A);

    private:
        std::type_index m_type_id;
        void* m_data;
        FuncStream_t m_ostream_func;
    };

    #include "inline/UserType.inl"
}

#endif