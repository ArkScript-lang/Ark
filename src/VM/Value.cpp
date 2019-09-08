#include <Ark/VM/Value.hpp>

#include <Ark/VM/Frame.hpp>
#include <Ark/Utils.hpp>

namespace Ark::internal
{
    Value::Value(ValueType type) :
        m_type(type), m_const(false)
    {
        if (m_type == ValueType::List)
            m_value = std::vector<Value>();
    }

    Value::Value(int value) :
        m_value(static_cast<double>(value)), m_type(ValueType::Number), m_const(false)
    {}

    Value::Value(float value) :
        m_value(static_cast<double>(value)), m_type(ValueType::Number), m_const(false)
    {}

    Value::Value(double value) :
        m_value(value), m_type(ValueType::Number), m_const(false)
    {}

    Value::Value(const std::string& value) :
        m_value(value), m_type(ValueType::String), m_const(false)
    {}

    Value::Value(std::string&& value) :
        m_value(value), m_type(ValueType::String), m_const(false)
    {}

    Value::Value(PageAddr_t value) :
        m_value(value), m_type(ValueType::PageAddr), m_const(false)
    {}

    Value::Value(NFT value) :
        m_value(value), m_type(ValueType::NFT), m_const(false)
    {}

    Value::Value(Value::ProcType value) :
        m_value(value), m_type(ValueType::CProc), m_const(false)
    {}

    Value::Value(std::vector<Value>&& value) :
        m_value(value), m_type(ValueType::List), m_const(false)
    {}

    Value::Value(Closure&& value) :
        m_value(value), m_type(ValueType::Closure), m_const(false)
    {}

    // --------------------------

    std::vector<Value>& Value::list()
    {
        return std::get<std::vector<Value>>(m_value);
    }

    Closure& Value::closure_ref()
    {
        return std::get<Closure>(m_value);
    }

    std::string& Value::string_ref()
    {
        return std::get<std::string>(m_value);
    }

    void Value::setConst(bool value)
    {
        m_const = value;
    }

    // --------------------------

    void Value::push_back(const Value& value)
    {
        m_type = ValueType::List;
        list().push_back(value);
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
            else if (nft == NFT::Undefined)
                os << "undefined";
            break;
        }

        case ValueType::CProc:
            os << "CProcedure";
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
            os << "Closure @ " << V.closure().pageAddr();
            break;
        
        default:
            os << "~\\._./~";
            break;
        }

        return os;
    }
}