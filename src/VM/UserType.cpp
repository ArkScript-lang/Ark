#include <Ark/VM/UserType.hpp>

namespace Ark
{
    void UserType::del()
    {
        // call a custom deleter on the data held by the usertype
        if (m_funcs != nullptr && m_funcs->deleter != nullptr)
            m_funcs->deleter(m_data);
    }

    bool operator==(const UserType& A, const UserType& B) noexcept
    {
        return (A.m_type_id == B.m_type_id) && (A.m_data == B.m_data);
    }

    bool operator<(const UserType& A, const UserType& B) noexcept
    {
        return false;
    }

    std::ostream& operator<<(std::ostream& os, const UserType& A) noexcept
    {
        if (A.m_funcs != nullptr && A.m_funcs->ostream_func != nullptr)
            return A.m_funcs->ostream_func(os, A);

        os << "UserType<" << A.m_type_id << ", 0x" << A.m_data << ">";
        return os;
    }
}