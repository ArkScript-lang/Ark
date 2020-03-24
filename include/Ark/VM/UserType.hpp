#ifndef ark_vm_usertype
#define ark_vm_usertype

#include <functional>
#include <iostream>
#include <vector>
#include <utility>
#include <typeindex>

namespace Ark
{
    /*
        A class to be handle to send (through C++ code) your own data types
        and retrieves them later one. A pointer to the value you want to store
        must be sent, thus the value must not be destroyed while the UserType lives,
        otherwise it would result in an UB when trying to use the object
    */
    class UserType
    {
    public:
        using FuncStream_t = std::function<std::ostream& (std::ostream& os, const UserType& A)>;

        template <typename T>
        explicit UserType(T* data=nullptr) :
            m_data(static_cast<void*>(data)),
            m_ostream_func(nullptr),
            m_type_id(std::type_index(typeid(T)))
        {}

        // setters
        inline void setOStream(FuncStream_t&& f);

        // getters
        inline const std::type_index type_id() const;
        inline void* data() const;

        // custom operators
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