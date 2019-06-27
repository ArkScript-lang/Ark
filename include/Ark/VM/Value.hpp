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
        using Value_t = std::variant<double, std::string, PageAddr_t, NFT, ProcType, Closure>;

        // TODO enhance
        Value(bool is_list=false);

        Value(int value);
        Value(double value);
        Value(const std::string& value);
        Value(PageAddr_t value);
        Value(NFT value);
        Value(Value::ProcType value);
        Value(const std::vector<Value>& value);
        Value(const Closure& value);
        Value(const Value& value);
        Value(const std::shared_ptr<Frame>& frame_ptr, PageAddr_t pa);

        ValueType valueType() const;

        double number() const;
        const std::string& string() const;
        PageAddr_t pageAddr() const;
        NFT nft() const;
        const ProcType proc() const;
        const std::vector<Value>& const_list() const;
        const Closure& closure() const;

        std::vector<Value>& list();
        Closure& closure_ref();

        void push_back(const Value& value);

        friend std::ostream& operator<<(std::ostream& os, const Value& V);
        friend inline bool operator==(const Value& A, const Value& B);

    private:
        Value_t m_value;
        ValueType m_type;
        std::vector<Value> m_list;
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