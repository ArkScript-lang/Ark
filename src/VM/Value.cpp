#include <Ark/VM/Value.hpp>

#include <Ark/VM/Frame.hpp>
#include <Ark/Utils.hpp>

namespace Ark::internal
{
    Value::Value(bool is_list) :
        m_type(ValueType::List)
    {}

    Value::Value(int value) :
        m_value(static_cast<double>(value)), m_type(ValueType::Number)
    {}

    Value::Value(double value) :
        m_value(value), m_type(ValueType::Number)
    {}

    Value::Value(const std::string& value) :
        m_value(value), m_type(ValueType::String)
    {}

    Value::Value(PageAddr_t value) :
        m_value(value), m_type(ValueType::PageAddr)
    {}

    Value::Value(NFT value) :
        m_value(value), m_type(ValueType::NFT)
    {}

    Value::Value(Value::ProcType value) :
        m_value(value), m_type(ValueType::CProc)
    {}

    Value::Value(const std::vector<Value>& value) :
        m_list(value), m_type(ValueType::List)
    {}

    Value::Value(const Closure& value) :
        m_value(value), m_type(ValueType::Closure)
    {}

    Value::Value(const Value& value) :
        m_value(value.m_value),
        m_list(value.m_list),
        m_type(value.m_type)
    {}

    Value::Value(const std::shared_ptr<Frame>& frame_ptr, PageAddr_t pa) :
        m_value(Closure(frame_ptr, pa)),
        m_type(ValueType::Closure)
    {}

    // --------------------------

    ValueType Value::valueType()
    {
        return m_type;
    }

    bool Value::isNumber() const
    {
        return m_type == ValueType::Number;
    }

    bool Value::isString() const
    {
        return m_type == ValueType::String;
    }

    bool Value::isPageAddr() const
    {
        return m_type == ValueType::PageAddr;
    }

    bool Value::isNFT() const
    {
        return m_type == ValueType::NFT;
    }

    bool Value::isProc() const
    {
        return m_type == ValueType::CProc;
    }

    bool Value::isList() const
    {
        return m_type == ValueType::List;
    }

    bool Value::isClosure() const
    {
        return m_type == ValueType::Closure;
    }

    // --------------------------

    double Value::number() const
    {
        return std::get<double>(m_value);
    }

    const std::string& Value::string() const
    {
        return std::get<std::string>(m_value);
    }

    PageAddr_t Value::pageAddr() const
    {
        return std::get<PageAddr_t>(m_value);
    }

    NFT Value::nft() const
    {
        return std::get<NFT>(m_value);
    }

    const Value::ProcType Value::proc() const
    {
        return std::get<Value::ProcType>(m_value);
    }

    const std::vector<Value>& Value::const_list() const
    {
        return m_list;
    }

    const Closure& Value::closure() const
    {
        return std::get<Closure>(m_value);
    }

    // --------------------------

    std::vector<Value>& Value::list()
    {
        return m_list;
    }

    Closure& Value::closure_ref()
    {
        return std::get<Closure>(m_value);
    }

    // --------------------------

    void Value::push_back(const Value& value)
    {
        m_is_list = true;
        m_list.push_back(value);
    }

    // --------------------------

    std::ostream& operator<<(std::ostream& os, const Value& V)
    {
        switch (V.valueType())
        {
        case ValueType::Number:
            os << Ark::Utils::toString(V.number());
            break;
        
        case ValueType::String:
            os << V.string();
            break;
        
        case ValueType::PageAddr:
            os << V.pageAddr();
            break;
        
        case ValueType::NFT:
        {
            NFT nft = V.nft();
            if (nft == NFT::Nil)
                os << "nil";
            else if (nft == NFT::False)
                os << "false";
            else if (nft == NFT::True)
                os << "true";
            break;
        }

        case ValueType::CProc:
            os << "Procedure";
            break;
        
        case ValueType::List:
        {
            os << "( ";
            for (auto& t: V.const_list())
                os << t << " ";
            os << ")";
            break;
        }

        case ValueType::Closure:
            os << "Closure (" << V.closure().frame() << ") @ " << V.closure().pageAddr();
            break;
        
        default:
            os << "~\\._./~";
            break;
        }

        return os;
    }
}