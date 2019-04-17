#include <Ark/VM/Value.hpp>

namespace Ark
{
    namespace VM
    {
        Value::Value() :
            m_is_list(false)
        {}

        template <> Value::Value<HugeNumber>(const HugeNumber& value) :
            m_value(value), m_is_list(false)
        {}

        template <> Value::Value<std::string>(const std::string& value) :
            m_value(value), m_is_list(false)
        {}

        template <> Value::Value<PageAddr_t>(const PageAddr_t& value) :
            m_value(value), m_is_list(false)
        {}

        template <> Value::Value<NFT>(const NFT& value) :
            m_value(value), m_is_list(false)
        {}

        template <> Value::Value<Ark::Lang::Node::ProcType>(const Ark::Lang::Node::ProcType& value) :
            m_value(value), m_is_list(false)
        {}

        template <> Value::Value<std::vector<Value>>(const std::vector<Value>& value) :
            m_list(value), m_is_list(true)
        {}

        template <> Value::Value<Value>(const Value& value) :
            m_value(value.m_value),
            m_list(value.m_list),
            m_is_list(value.m_is_list)
        {}

        Value::~Value()
        {}

        Value& Value::operator=(const Value& value)
        {
            if (this == &value)
                return *this;
            
            m_value = value.m_value;
            m_list = value.m_list;
            m_is_list = value.m_is_list;

            return *this;
        }

        bool Value::isNumber() const
        {
            return !m_is_list && std::holds_alternative<HugeNumber>(m_value);
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
            return !m_is_list && std::holds_alternative<Ark::Lang::Node::ProcType>(m_value);
        }

        bool Value::isList() const
        {
            return m_is_list;
        }

        const HugeNumber& Value::number() const
        {
            return std::get<HugeNumber>(m_value);
        }

        const std::string& Value::string() const
        {
            return std::get<std::string>(m_value);
        }

        const PageAddr_t Value::pageAddr() const
        {
            return std::get<PageAddr_t>(m_value);
        }

        const NFT Value::nft() const
        {
            return std::get<NFT>(m_value);
        }

        const Ark::Lang::Node::ProcType Value::proc() const
        {
            return std::get<Ark::Lang::Node::ProcType>(m_value);
        }

        const std::vector<Value>& Value::list() const
        {
            return m_list;
        }
    }
}