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
    class Frame;

    class Value
    {
    public:
        using ProcType  = Value(*)(const std::vector<Value>&);
        using Iterator = std::vector<Value>::const_iterator;
        using ValueType = std::variant<double, std::string, PageAddr_t, NFT, ProcType, Closure>;

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
        Value(std::shared_ptr<Frame> frame_ptr, PageAddr_t pa);

        bool isNumber() const;
        bool isString() const;
        bool isPageAddr() const;
        bool isNFT() const;
        bool isProc() const;
        bool isList() const;
        bool isClosure() const;

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

        inline std::string typeToString() const
        {
            // must have the same order as the variant on L26
            static const std::vector<std::string> types_str = { "Number", "String", "PageAddr", "Symbol", "Procedure", "Closure" };
            
            if (isNFT())
            {
                if (nft() == NFT::Nil)
                    return "Nil";
                return "Bool";
            }

            return types_str[m_value.index()];
        }

    private:
        ValueType m_value;
        std::vector<Value> m_list;
        bool m_is_list;
    };

    inline bool operator==(const Value& A, const Value& B)
    {
        // values should have the same type
        if (A.m_value.index() != B.m_value.index())
            return false;

        if (A.m_is_list)
            throw Ark::TypeError("Can not compare lists");
        
        return A.m_value == B.m_value;
    }
}

#endif