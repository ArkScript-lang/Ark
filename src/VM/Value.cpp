#include <Ark/VM/Value.hpp>

#include <Ark/VM/Frame.hpp>
#include <Ark/Utils.hpp>

namespace Ark::internal
{
    Value::Value(bool is_list) :
        m_is_list(is_list)
    {}

    Value::Value(int value) :
        m_value(static_cast<double>(value)), m_is_list(false)
    {}

    Value::Value(double value) :
        m_value(value), m_is_list(false)
    {}

    Value::Value(const std::string& value) :
        m_value(value), m_is_list(false)
    {}

    Value::Value(PageAddr_t value) :
        m_value(value), m_is_list(false)
    {}

    Value::Value(NFT value) :
        m_value(value), m_is_list(false)
    {}

    Value::Value(Value::ProcType value) :
        m_value(value), m_is_list(false)
    {}

    Value::Value(const std::vector<Value>& value) :
        m_list(value), m_is_list(true)
    {}

    Value::Value(const Closure& value) :
        m_value(value), m_is_list(false)
    {}

    Value::Value(const Value& value) :
        m_value(value.m_value),
        m_list(value.m_list),
        m_is_list(value.m_is_list)
    {}

    Value::Value(std::shared_ptr<Frame> frame_ptr, PageAddr_t pa) :
        m_value(Closure(frame_ptr, pa)),
        m_is_list(false)
    {}

    // --------------------------

    bool Value::isNumber() const
    {
        return !m_is_list && std::holds_alternative<double>(m_value);
    }

    bool Value::isString() const
    {
        return !m_is_list && std::holds_alternative<std::string>(m_value);
    }

    bool Value::isPageAddr() const
    {
        return !m_is_list && std::holds_alternative<PageAddr_t>(m_value);
    }

    bool Value::isNFT() const
    {
        return !m_is_list && std::holds_alternative<NFT>(m_value);
    }

    bool Value::isProc() const
    {
        return !m_is_list && std::holds_alternative<Value::ProcType>(m_value);
    }

    bool Value::isList() const
    {
        return m_is_list;
    }

    bool Value::isClosure() const
    {
        return !m_is_list && std::holds_alternative<Closure>(m_value);
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
        if (V.isNumber())
            os << Ark::Utils::toString(V.number());
        else if (V.isString())
            os << V.string();
        else if (V.isPageAddr())
            os << V.pageAddr();
        else if (V.isNFT())
        {
            NFT nft = V.nft();
            if (nft == NFT::Nil)
                os << "nil";
            else if (nft == NFT::False)
                os << "false";
            else if (nft == NFT::True)
                os << "true";
        }
        else if (V.isProc())
            os << "Procedure";
        else if (V.isList())
        {
            os << "( ";
            for (auto& t: V.const_list())
                os << t << " ";
            os << ")";
        }
        else if (V.isClosure())
            os << "Closure (" << V.closure().frame() << ") @ " << V.closure().pageAddr();
        return os;
    }
}