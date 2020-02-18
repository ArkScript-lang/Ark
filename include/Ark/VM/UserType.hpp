#ifndef ark_vm_usertype
#define ark_vm_usertype

#include <functional>
#include <iostream>
#include <vector>
#include <utility>
#include <typeindex>

namespace Ark
{
    class UserType
    {
    public:
        using FuncStream_t = std::function<std::ostream& (std::ostream& os, const UserType& A)>;

        template <typename T>
        explicit UserType(T* data=nullptr) :
            m_data(static_cast<void*>(data)),
            m_ostream_func(nullptr),
            m_type_id(std::type_index(typeid(data)))
        {
        }

        inline void setOStream(FuncStream_t&& f)
        {
            m_ostream_func = std::move(f);
        }

        inline const std::type_index type_id() const
        {
            return m_type_id;
        }

        inline void* data() const
        {
            return m_data;
        }

        // custom operators
        inline bool not_() const
        {
            // TODO let the user implement his/her own
            return false;
        }

        friend inline bool operator==(const UserType& A, const UserType& B);
        friend inline bool operator<(const UserType& A, const UserType& B);
        friend inline std::ostream& operator<<(std::ostream& os, const UserType& A);
    
    private:
        std::type_index m_type_id;
        void* m_data;
        FuncStream_t m_ostream_func;
    };

    inline bool operator==(const UserType& A, const UserType& B)
    {
        return (A.m_type_id == B.m_type_id) && (A.m_data == B.m_data);
    }

    inline bool operator<(const UserType& A, const UserType& B)
    {
        return false;
    }

    inline std::ostream& operator<<(std::ostream& os, const UserType& A)
    {
        if (A.m_ostream_func != nullptr)
            return A.m_ostream_func(os, A);
        
        os << "UserType<" << A.m_type_id.hash_code() << ", 0x" << A.m_data << ">";
        return os;
    }
}

#endif