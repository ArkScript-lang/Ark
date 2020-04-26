#include <Ark/VM/Value.hpp>

#include <Ark/VM/Frame.hpp>
#include <Ark/Utils.hpp>

namespace Ark::internal
{
    Value::Value() :
        m_type(ValueType::Undefined)
    {}

    // --------------------------

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

    Value::Value(Value::ProcType value) :
        m_value(value), m_type(ValueType::CProc), m_const(false)
    {}

    Value::Value(std::vector<Value>&& value) :
        m_value(value), m_type(ValueType::List), m_const(false)
    {}

    Value::Value(Closure&& value) :
        m_value(value), m_type(ValueType::Closure), m_const(false)
    {}

    Value::Value(UserType&& value) :
        m_value(value), m_type(ValueType::User), m_const(false)
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

    UserType& Value::usertype_ref()
    {
        return std::get<UserType>(m_value);
    }

    // --------------------------

    void Value::push_back(const Value& value)
    {
        m_type = ValueType::List;
        list().push_back(value);
    }

    void Value::push_back(Value&& value)
    {
        m_type = ValueType::List;
        list().push_back(value);
    }

    // --------------------------

    void Value::registerVM(Ark::VM* vm)
    {
        m_vm = vm;
    }

    // --------------------------

    int dec_places(double d)
    {
        constexpr double precision = 1e-7;
        double temp = 0.0;
        int decimal_places = 0;

        do
        {
            d *= 10;
            temp = d - static_cast<int>(d);
            decimal_places++;
        } while(temp > precision && decimal_places < std::numeric_limits<double>::digits10);
        
        return decimal_places;
    }

    int dig_places(double d)
    {
        int digit_places = 0;
        int i = static_cast<int>(d);
        while (i != 0)
        {
            digit_places++;
            i /= 10;
        }
        return digit_places;
    }

    std::ostream& operator<<(std::ostream& os, const Value& V)
    {
        switch (V.m_type)
        {
        case ValueType::Number:
        {
            double d = V.number();
            os.precision(dig_places(d) + dec_places(d));
            os << d;
            break;
        }
        
        case ValueType::String:
            os << V.string();
            break;
        
        case ValueType::PageAddr:
            os << "Function @ " << V.pageAddr();
            break;

        case ValueType::CProc:
            os << "CProcedure";
            break;
        
        case ValueType::List:
        {
            os << "[";
            for (auto it=V.const_list().begin(), it_end=V.const_list().end(); it != it_end; ++it)
            {
                if (it->m_type == ValueType::String)
                    os << "\"" << (*it) << "\"";
                else
                    os << (*it);
                if (it + 1 != it_end)
                    os << " ";
            }
            os << "]";
            break;
        }

        case ValueType::Closure:
            os << V.closure();
            break;
        
        case ValueType::User:
            os << V.usertype();
            break;
        
        case ValueType::Nil:
            os << "nil";
            break;
        
        case ValueType::True:
            os << "true";
            break;
        
        case ValueType::False:
            os << "false";
            break;
        
        case ValueType::Undefined:
            os << "undefined";
            break;
        
        default:
            os << "~\\._./~";
            break;
        }

        return os;
    }
}