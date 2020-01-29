#ifndef ark_vm_usertype
#define ark_vm_usertype

#include <functional>
#include <iostream>

namespace Ark
{
    class UserType
    {
    public:
        using FuncStream_t = std::function<std::ostream& (std::ostream& os, const UserType& A)>;

        template <typename T>
        UserType(unsigned type_id, T* data=nullptr) :
            m_type_id(type_id),
            m_data(static_cast<void*>(data)),
            m_ostream_func(nullptr)
        {}

        inline void setOStream(FuncStream_t&& f)
        {
            m_ostream_func = std::move(f);
        }

        inline const unsigned type_id() const
        {
            return m_type_id;
        }

        inline void* data() const
        {
            return m_data;
        }

        friend inline bool operator==(const UserType& A, const UserType& B);
        friend inline bool operator<(const UserType& A, const UserType& B);
        friend inline std::ostream& operator<<(std::ostream& os, const UserType& A);
    
    private:
        unsigned m_type_id;
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
        
        os << "UserType<" << A.m_type_id << ", 0x" << A.m_data << ">";
        return os;
    }
}

#endif