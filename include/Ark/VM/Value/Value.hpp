/**
 * @file Value.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Default value type handled by the virtual machine
 * @date 2024-04-20
 *
 * @copyright Copyright (c) 2020-2026
 *
 */

#ifndef ARK_VM_VALUE_HPP
#define ARK_VM_VALUE_HPP

#include <vector>
#include <variant>
#include <string>
#include <cinttypes>
#include <array>
#include <memory>
#include <type_traits>

#include <Ark/VM/Value/Closure.hpp>
#include <Ark/VM/Value/UserType.hpp>
#include <Ark/VM/Value/Procedure.hpp>
#include <Ark/Utils/Platform.hpp>

namespace Ark
{
    // Order is important because we are doing some optimizations to check ranges
    // of types based on their integer values.
    enum class ValueType : unsigned
    {
        List = 0,
        Number = 1,
        String = 2,
        PageAddr = 3,
        CProc = 4,
        Closure = 5,
        User = 6,
        Dict = 7,

        Nil = 8,
        True = 9,
        False = 10,
        Undefined = 11,
        Reference = 12,
        InstPtr = 13,

        Any = 14  ///< Used only for typechecking
    };

    constexpr std::array types_to_str = {
        "List",
        "Number",
        "String",
        "Function",
        "CProc",
        "Closure",
        "UserType",
        "Dict",
        "Nil",
        "Bool",
        "Bool",
        "Undefined",
        "Reference",
        "InstPtr",
        "Any"
    };
}

template <>
struct std::hash<Ark::ValueType>
{
    [[nodiscard]] std::size_t operator()(const Ark::ValueType& s) const noexcept
    {
        return std::hash<std::underlying_type_t<Ark::ValueType>> {}(static_cast<std::underlying_type_t<Ark::ValueType>>(s));
    }
};

namespace Ark
{
    class VM;
    class BytecodeReader;

    namespace internal
    {
        class Dict;
    }

    class ARK_API Value
    {
    public:
        using Number_t = double;
        using String_t = std::string;
        using List_t = std::vector<Value>;
        using Ref_t = Value*;
        using Dict_t = internal::Dict;

        using Value_t = std::variant<
            Number_t,                 //  8 bytes
            String_t,                 // 32 bytes
            internal::PageAddr_t,     //  2 bytes
            Procedure,                // 32 bytes
            internal::Closure,        // 24 bytes
            UserType,                 // 24 bytes
            List_t,                   // 24 bytes
            std::shared_ptr<Dict_t>,  // 32 bytes
            Ref_t                     //  8 bytes
            >;                        // +8 bytes overhead
        //                      total 40 bytes

        /**
         * @brief Construct a new Value object
         *
         */
        Value() noexcept;

        /**
         * @brief Construct a new Value object
         *
         * @param type the value type which is going to be held
         */
        explicit Value(ValueType type) noexcept;

        /**
         * @brief Construct a new Value object
         * @details Use at your own risks. Asking for a value type N and putting a non-matching value
         *          will result in errors at runtime.
         *
         * @tparam T
         * @param type value type wanted
         * @param value value needed
         */
        template <typename T>
        Value(const ValueType type, T&& value) noexcept :
            m_type(type), m_value(value)
        {}

        explicit Value(int value) noexcept;
        explicit Value(double value) noexcept;
        explicit Value(const String_t& value) noexcept;
        explicit Value(const char* value) noexcept;
        explicit Value(internal::PageAddr_t value) noexcept;
        explicit Value(Procedure&& value) noexcept;
        explicit Value(List_t&& value) noexcept;
        explicit Value(internal::Closure&& value) noexcept;
        explicit Value(UserType&& value) noexcept;
        explicit Value(Dict_t&& value) noexcept;
        explicit Value(Ref_t ref) noexcept;

        [[nodiscard]] ValueType valueType() const noexcept { return m_type; }
        [[nodiscard]] bool isFunction() const noexcept
        {
            return m_type == ValueType::PageAddr || m_type == ValueType::Closure || m_type == ValueType::CProc ||
                (m_type == ValueType::Reference && reference()->isFunction());
        }
        [[nodiscard]] bool isIndexable() const noexcept
        {
            return m_type == ValueType::List || m_type == ValueType::String;
        }

        [[nodiscard]] Number_t number() const { return std::get<Number_t>(m_value); }

        [[nodiscard]] const String_t& string() const { return std::get<String_t>(m_value); }
        [[nodiscard]] String_t& stringRef() { return std::get<String_t>(m_value); }


        [[nodiscard]] const List_t& constList() const { return std::get<List_t>(m_value); }
        [[nodiscard]] List_t& list() { return std::get<List_t>(m_value); }

        [[nodiscard]] const UserType& usertype() const { return std::get<UserType>(m_value); }
        [[nodiscard]] UserType& usertypeRef() { return std::get<UserType>(m_value); }

        [[nodiscard]] const Dict_t& dict() const { return *std::get<std::shared_ptr<Dict_t>>(m_value); }
        [[nodiscard]] Dict_t& dictRef() { return *std::get<std::shared_ptr<Dict_t>>(m_value); }

        [[nodiscard]] Ref_t reference() const { return std::get<Ref_t>(m_value); }

        [[nodiscard]] internal::PageAddr_t pageAddr() const { return std::get<internal::PageAddr_t>(m_value); }

        /**
         * @brief Add an element to the list held by the value (if the value type is set to list)
         *
         * @param value
         */
        void push_back(const Value& value);

        /**
         * @brief Add an element to the list held by the value (if the value type is set to list)
         *
         * @param value
         */
        void push_back(Value&& value);

        std::string toString(VM& vm, bool show_as_code = false) const noexcept;

        friend ARK_API bool operator==(const Value& A, const Value& B) noexcept;
        friend ARK_API_INLINE bool operator<(const Value& A, const Value& B) noexcept;
        friend ARK_API_INLINE bool operator!(const Value& A) noexcept;

        friend class Ark::VM;
        friend class Ark::BytecodeReader;
        friend struct std::hash<Ark::Value>;

    private:
        ValueType m_type;
        Value_t m_value;

        [[nodiscard]] constexpr uint8_t typeNum() const noexcept { return static_cast<uint8_t>(m_type); }

        [[nodiscard]] const Procedure& proc() const { return std::get<Procedure>(m_value); }
        [[nodiscard]] const internal::Closure& closure() const { return std::get<internal::Closure>(m_value); }
        [[nodiscard]] internal::Closure& refClosure() { return std::get<internal::Closure>(m_value); }
    };

    inline bool operator<(const Value& A, const Value& B) noexcept
    {
        if (A.m_type != B.m_type)
            return (A.typeNum() - B.typeNum()) < 0;
        return A.m_value < B.m_value;
    }

    inline bool operator!=(const Value& A, const Value& B) noexcept
    {
        return !(A == B);
    }

    inline bool operator!(const Value& A) noexcept
    {
        switch (A.valueType())
        {
            case ValueType::List:
                return A.constList().empty();

            case ValueType::Number:
                return A.number() == 0.0;

            case ValueType::String:
                return A.string().empty();

            case ValueType::User:
                [[fallthrough]];
            case ValueType::Nil:
                [[fallthrough]];
            case ValueType::False:
                return true;

            case ValueType::True:
                [[fallthrough]];
            default:
                return false;
        }
    }
}

namespace std
{
    [[nodiscard]] inline std::string to_string(const Ark::ValueType type) noexcept
    {
        const auto index = static_cast<std::underlying_type_t<Ark::ValueType>>(type);
        return Ark::types_to_str[index];
    }
}

template <>
struct std::hash<std::vector<Ark::Value>>
{
    [[nodiscard]] std::size_t operator()(const std::vector<Ark::Value>& s) const noexcept
    {
        return std::hash<const Ark::Value*> {}(s.data());
    }
};

template <>
struct std::hash<std::shared_ptr<Ark::Value::Dict_t>>
{
    [[nodiscard]] std::size_t operator()(const std::shared_ptr<Ark::Value::Dict_t>& s) const noexcept
    {
        return std::hash<const Ark::Value::Dict_t*> {}(s.get());
    }
};

template <>
struct std::hash<Ark::Value>
{
    [[nodiscard]] std::size_t operator()(const Ark::Value& s) const noexcept
    {
        return std::hash<Ark::Value::Value_t> {}(s.m_value);
    }
};

#endif
