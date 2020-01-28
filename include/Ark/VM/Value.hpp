#ifndef ark_vm_value
#define ark_vm_value

#include <vector>
#include <variant>
#include <string>
#include <cinttypes>
#include <iostream>
#include <memory>
#include <functional>

#include <Ark/VM/Types.hpp>
#include <Ark/VM/Closure.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/VM/UserType.hpp>

namespace Ark
{
    template<bool D> class VM_t;
}

namespace Ark::internal
{
    enum class ValueType
    {
        List,
        Number,
        String,
        PageAddr,
        NFT,
        CProc,
        Closure,
        User
    };

    class Frame;

    class Value
    {
    public:
        using ProcType = std::function<Value (const std::vector<Value>&)>;
        using Iterator = std::vector<Value>::const_iterator;
        using Value_t = std::variant<double, std::string, PageAddr_t, NFT, ProcType, Closure, UserType, std::vector<Value>>;

        Value() = default;
        Value(Value&&) = default;
        // TODO maybe a bad idea, noticed that thingy = FFI::nil does not work properly
        Value(const Value&) = default;
        Value& operator=(const Value&) = default;

        Value(ValueType type);
        Value(int value);
        Value(float value);
        Value(double value);
        Value(const std::string& value);
        Value(std::string&& value);
        Value(PageAddr_t value);
        Value(NFT value);
        Value(Value::ProcType value);
        Value(std::vector<Value>&& value);
        Value(Closure&& value);
        Value(UserType&& value);

        inline ValueType valueType() const
        {
            return m_type;
        }

        inline bool isConst() const
        {
            return m_const;
        }

        inline double number() const
        {
            return std::get<double>(m_value);
        }

        inline const std::string& string() const
        {
            return std::get<std::string>(m_value);
        }

        inline const std::vector<Value>& const_list() const
        {
            return std::get<std::vector<Value>>(m_value);
        }

        inline const UserType& usertype() const
        {
            return std::get<UserType>(m_value);
        }

        std::vector<Value>& list();
        std::string& string_ref();
        UserType& usertype_ref();

        void push_back(const Value& value);
        void push_back(Value&& value);

        friend std::ostream& operator<<(std::ostream& os, const Value& V);
        friend inline bool operator==(const Value& A, const Value& B);
        friend inline bool operator<(const Value& A, const Value& B);

        template<bool D> friend class Ark::VM_t;

    private:
        Value_t m_value;
        ValueType m_type;
        bool m_const;

        inline PageAddr_t pageAddr() const
        {
            return std::get<PageAddr_t>(m_value);
        }

        inline NFT nft() const
        {
            return std::get<NFT>(m_value);
        }

        inline const ProcType& proc() const
        {
            return std::get<Value::ProcType>(m_value);
        }

        inline const Closure& closure() const
        {
            return std::get<Closure>(m_value);
        }

        void setConst(bool value);
        Closure& closure_ref();
    };

    inline bool operator==(const Value::ProcType& f, const Value::ProcType& g)
    {
        return f.template target<Value (const std::vector<Value>&)>() == g.template target<Value (const std::vector<Value>&)>();
    }

    inline bool operator==(const Value& A, const Value& B)
    {
        // values should have the same type
        if (A.m_type != B.m_type)
            return false;
        
        return A.m_value == B.m_value;
    }

    inline bool operator<(const Value& A, const Value& B)
    {
        if (A.m_type != B.m_type)
            return (static_cast<int>(A.m_type) - static_cast<int>(B.m_type)) < 0;
        return A.m_value < B.m_value;
    }

    inline bool operator!=(const Value& A, const Value& B)
    {
        return !(A == B);
    }
}

#endif