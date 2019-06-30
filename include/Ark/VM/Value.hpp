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

        ValueType valueType() const;
        bool isConst() const;

        double number() const;
        const std::string& string() const;
        PageAddr_t pageAddr() const;
        NFT nft() const;
        const ProcType proc() const;
        const std::vector<Value>& const_list() const;
        const Closure& closure() const;

        std::vector<Value>& list();
        Closure& closure_ref();
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
        // don't compare lists
        if (A.m_type == ValueType::List)
            return false;
        
        return A.m_value == B.m_value;
    }
}

#endif