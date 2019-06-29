#include <Ark/VM/Value.hpp>

#include <Ark/VM/Frame.hpp>
#include <Ark/Utils.hpp>

namespace Ark::internal
{
    Value::Value(ValueType type) :
        m_type(type)
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

    Value::Value(std::string&& value) :
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

    Value::Value(std::vector<Value>&& value) :
        m_value(value), m_type(ValueType::List)
    {}

    Value::Value(Closure&& value) :
        m_value(value), m_type(ValueType::Closure)
    {}

    // --------------------------

    ValueType Value::valueType() const
    {
        return m_type;
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
        return std::get<std::vector<Value>>(m_value);
    }

    const Closure& Value::closure() const
    {
        return std::get<Closure>(m_value);
    }

    // --------------------------

    std::vector<Value>& Value::list()
    {
        return std::get<std::vector<Value>>(m_value);
    }

    Closure& Value::closure_ref()
    {
        return std::get<Closure>(m_value);
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