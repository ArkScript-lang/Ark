#include <Ark/VM/Value.hpp>

#include <Ark/VM/Frame.hpp>
#include <Ark/Utils.hpp>

namespace Ark::internal
{
    Value::Value() :
        m_type(ValueType::Undefined)
    {}

    Value::Value(const Value& value) :
        m_type(value.m_type), m_const(value.m_const), m_vm(value.m_vm)
    {
        switch (m_type)
        {
            case ValueType::List:
                new (&m_value.list) std::vector<Value>;
                m_value.list = value.m_value.list;
                break;

            case ValueType::Number:
                m_value.number = value.m_value.number;
                break;

            case ValueType::String:
                new (&m_value.string) std::string;
                m_value.string = value.m_value.string;
                break;

            case ValueType::PageAddr:
                m_value.page = value.m_value.page;
                break;

            case ValueType::CProc:
                m_value.proc = value.m_value.proc;
                break;

            case ValueType::Closure:
                new (&m_value.closure) Closure;
                m_value.closure = value.m_value.closure;
                break;

            case ValueType::User:
                new (&m_value.user) UserType;
                m_value.user = value.m_value.user;
                break;

            default:
                break;
        }
    }

    Value& Value::operator=(const Value& value)
    {
        this->~Value();

        m_type = value.m_type;
        m_const = value.m_const;
        m_vm = value.m_vm;

        switch (m_type)
        {
            case ValueType::List:
                new (&m_value.list) std::vector<Value>;
                m_value.list = value.m_value.list;
                break;

            case ValueType::Number:
                m_value.number = value.m_value.number;
                break;

            case ValueType::String:
                new (&m_value.string) std::string;
                m_value.string = value.m_value.string;
                break;

            case ValueType::PageAddr:
                m_value.page = value.m_value.page;
                break;

            case ValueType::CProc:
                m_value.proc = value.m_value.proc;
                break;

            case ValueType::Closure:
                new (&m_value.closure) Closure;
                m_value.closure = value.m_value.closure;
                break;

            case ValueType::User:
                new (&m_value.user) UserType;
                m_value.user = value.m_value.user;
                break;

            default:
                break;
        }

        return *this;
    }

    Value::~Value()
    {
        switch (m_type)
        {
            case ValueType::List:
                m_value.list.~vector<Value>();
                break;

            case ValueType::String:
                m_value.string.~basic_string();
                break;

            case ValueType::Closure:
                m_value.closure.~Closure();
                break;

            case ValueType::User:
                m_value.user.~UserType();
                break;

            default:
                break;
        }
    }

    // --------------------------

    Value::Value(ValueType type) :
        m_type(type), m_const(false)
    {
        switch (m_type)
        {
            case ValueType::List:
                new (&m_value.list) std::vector<Value>;
                break;

            case ValueType::String:
                new (&m_value.string) std::string;
                break;

            case ValueType::Closure:
                new (&m_value.closure) Closure;
                break;

            case ValueType::User:
                new (&m_value.user) UserType;
                break;
        }
    }

    Value::Value(int value) :
        m_type(ValueType::Number), m_const(false)
    {
        m_value.number = static_cast<double>(value);
    }

    Value::Value(float value) :
        m_type(ValueType::Number), m_const(false)
    {
        m_value.number = static_cast<double>(value);
    }

    Value::Value(double value) :
        m_type(ValueType::Number), m_const(false)
    {
        m_value.number = static_cast<double>(value);
    }

    Value::Value(const std::string& value) :
        m_type(ValueType::String), m_const(false)
    {
        new (&m_value.string) String;
        m_value.string = value;
    }

    Value::Value(const String& value) :
        m_type(ValueType::String), m_const(false)
    {
        new (&m_value.string) String;
        m_value.string = value;
    }

    Value::Value(const char* value) :
        m_type(ValueType::String), m_const(false)
    {
        new (&m_value.string) String;
        m_value.string = value;
    }

    Value::Value(PageAddr_t value) :
        m_type(ValueType::PageAddr), m_const(false)
    {
        m_value.page = value;
    }

    Value::Value(Value::ProcType value) :
        m_type(ValueType::CProc), m_const(false)
    {
        m_value.proc = value;
    }

    Value::Value(std::vector<Value>&& value) :
        m_type(ValueType::List), m_const(false)
    {
        new (&m_value.list) std::vector<Value>;
        m_value.list = value;
    }

    Value::Value(Closure&& value) :
        m_type(ValueType::Closure), m_const(false)
    {
        new (&m_value.closure) Closure;
        m_value.closure = value;
    }

    Value::Value(UserType&& value) :
        m_type(ValueType::User), m_const(false)
    {
        new (&m_value.user) UserType;
        m_value.user = value;
    }

    // --------------------------

    std::vector<Value>& Value::list()
    {
        return m_value.list;
    }

    Closure& Value::closure_ref()
    {
        return m_value.closure;
    }

    String& Value::string_ref()
    {
        return m_value.string;
    }

    UserType& Value::usertype_ref()
    {
        return m_value.user;
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
            os << V.string().c_str();
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