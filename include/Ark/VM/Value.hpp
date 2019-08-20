#ifndef ark_vm_value
#define ark_vm_value

#include <vector>
#include <variant>
#include <string>
#include <cinttypes>
#include <iostream>
#include <memory>

#include <Ark/VM/Types.hpp>
#include <Ark/VM/Closure.hpp>
#include <Ark/Exceptions.hpp>

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
        Closure
    };

    class Frame;

    class Value
    {
    public:
        using ProcType  = Value(*)(const std::vector<Value>&);
        using Iterator = std::vector<Value>::const_iterator;
        using Value_t = std::variant<double, std::string, PageAddr_t, NFT, ProcType, Closure, std::vector<Value>>;

        Value() = default;
        Value(Value&&) = default;
        Value(const Value&) = default;
        Value& operator=(const Value&) = default;

        Value(ValueType type);
        Value(int value);
        Value(double value);
        Value(const std::string& value);
        Value(std::string&& value);
        Value(PageAddr_t value);
        Value(NFT value);
        Value(Value::ProcType value);
        Value(std::vector<Value>&& value);
        Value(Closure&& value);

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

        inline PageAddr_t pageAddr() const
        {
            return std::get<PageAddr_t>(m_value);
        }

        inline NFT nft() const
        {
            return std::get<NFT>(m_value);
        }

        inline const ProcType proc() const
        {
            return std::get<Value::ProcType>(m_value);
        }

        inline const std::vector<Value>& const_list() const
        {
            return std::get<std::vector<Value>>(m_value);
        }

        inline const Closure& closure() const
        {
            return std::get<Closure>(m_value);
        }

        std::vector<Value>& list();
        Closure& closure_ref();
        std::string& string_ref();
        void setConst(bool value);

        void push_back(const Value& value);
        void push_back(Value&& value);

        friend std::ostream& operator<<(std::ostream& os, const Value& V);
        friend inline bool operator==(const Value& A, const Value& B);

    private:
        Value_t m_value;
        ValueType m_type;
        bool m_const;
    };

    inline bool operator==(const Value& A, const Value& B)
    {
        // values should have the same type
        if (A.m_type != B.m_type)
            return false;
        
        return A.m_value == B.m_value;
    }

    inline bool operator!=(const Value& A, const Value& B)
    {
        return !(A == B);
    }
}

#endif