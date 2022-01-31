#include <Ark/VM/Value/UserType.hpp>

namespace Ark
{
    void UserType::del()
    {
        if (m_funcs && m_funcs->deleter)
            m_funcs->deleter(m_data);
    }

    bool operator==(const UserType& A, const UserType& B) noexcept
    {
        return (A.m_type_id == B.m_type_id) && (A.m_data == B.m_data);
    }

    bool operator<(const UserType& A [[maybe_unused]], const UserType& B [[maybe_unused]]) noexcept
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
