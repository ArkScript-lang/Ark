#ifndef ark_vm_usertype
#define ark_vm_usertype

namespace Ark
{
    class UserType
    {
    public:
        template <typename T>
        UserType(unsigned type_id, T* data=nullptr) :
            m_type_id(type_id),
            data(static_cast<void*>(data))
        {}

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
    
    private:
        unsigned m_type_id;
        void* m_data;
    };

    inline bool operator==(const UserType& A, const UserType& B)
    {
        return (A.m_type_id == B.m_type_id) && (A.m_data == B.m_data);
    }

    inline bool operator<(const UserType& A, const UserType& B)
    {
        return false;
    }
}

#endif