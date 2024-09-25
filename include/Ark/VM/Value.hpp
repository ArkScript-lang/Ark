/**
 * @file Value.hpp
 * @author Default value type handled by the virtual machine
 * @brief
 * @version 1.2
 * @date 2024-04-20
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef ARK_VM_VALUE_HPP
#define ARK_VM_VALUE_HPP

#include <vector>
#include <variant>
#include <string>
#include <cinttypes>
#include <array>

#include <Ark/VM/Value/Closure.hpp>
#include <Ark/VM/Value/UserType.hpp>
#include <Ark/Platform.hpp>

namespace Ark
{
    class VM;
    class BytecodeReader;

    // Order is important because we are doing some optimizations to check ranges
    // of types based on their integer values.
    enum class ValueType
    {
        List = 0,
        Number = 1,
        String = 2,
        PageAddr = 3,
        CProc = 4,
        Closure = 5,
        User = 6,

        Nil = 7,
        True = 8,
        False = 9,
        Undefined = 10,
        Reference = 11,
        InstPtr = 12,

        Any = 99  ///< Used only for typechecking
    };

    const std::array<std::string, 13> types_to_str = {
        "List", "Number", "String", "Function",
        "CProc", "Closure", "UserType", "Nil",
        "Bool", "Bool", "Undefined", "Reference",
        "InstPtr"
    };

    class ARK_API Value
    {
    public:
        using ProcType = Value (*)(std::vector<Value>&, VM*);
        using Iterator = std::vector<Value>::iterator;

        using Value_t = std::variant<
            double,                //  8 bytes
            std::string,           // 32 bytes
            internal::PageAddr_t,  //  2 bytes
            ProcType,              //  8 bytes
            internal::Closure,     // 24 bytes
            UserType,              // 24 bytes
            std::vector<Value>,    // 24 bytes
            Value*                 //  8 bytes
            >;                     // +8 bytes overhead
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
        explicit Value(float value) noexcept;
        explicit Value(double value) noexcept;
        explicit Value(const std::string& value) noexcept;
        explicit Value(const char* value) noexcept;
        explicit Value(internal::PageAddr_t value) noexcept;
        explicit Value(ProcType value) noexcept;
        explicit Value(std::vector<Value>&& value) noexcept;
        explicit Value(internal::Closure&& value) noexcept;
        explicit Value(UserType&& value) noexcept;
        explicit Value(Value* ref) noexcept;

        [[nodiscard]] ValueType valueType() const noexcept { return m_type; }
        [[nodiscard]] bool isFunction() const noexcept
        {
            const auto type = valueType();
            return type == ValueType::PageAddr || type == ValueType::Closure || type == ValueType::CProc ||
                (type == ValueType::Reference && reference()->isFunction());
        }

        [[nodiscard]] double number() const { return std::get<double>(m_value); }
        [[nodiscard]] const std::string& string() const { return std::get<std::string>(m_value); }
        [[nodiscard]] const std::vector<Value>& constList() const { return std::get<std::vector<Value>>(m_value); }
        [[nodiscard]] const UserType& usertype() const { return std::get<UserType>(m_value); }
        [[nodiscard]] std::vector<Value>& list() { return std::get<std::vector<Value>>(m_value); }
        [[nodiscard]] std::string& stringRef() { return std::get<std::string>(m_value); }
        [[nodiscard]] UserType& usertypeRef() { return std::get<UserType>(m_value); }
        [[nodiscard]] Value* reference() const { return std::get<Value*>(m_value); }

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

        std::string toString(VM& vm) const noexcept;

        friend ARK_API_INLINE bool operator==(const Value& A, const Value& B) noexcept;
        friend ARK_API_INLINE bool operator<(const Value& A, const Value& B) noexcept;
        friend ARK_API_INLINE bool operator!(const Value& A) noexcept;

        friend class Ark::VM;
        friend class Ark::BytecodeReader;

    private:
        ValueType m_type;
        Value_t m_value;

        [[nodiscard]] constexpr uint8_t typeNum() const noexcept { return static_cast<uint8_t>(m_type); }

        [[nodiscard]] internal::PageAddr_t pageAddr() const { return std::get<internal::PageAddr_t>(m_value); }
        [[nodiscard]] const ProcType& proc() const { return std::get<ProcType>(m_value); }
        [[nodiscard]] const internal::Closure& closure() const { return std::get<internal::Closure>(m_value); }
        [[nodiscard]] internal::Closure& refClosure() { return std::get<internal::Closure>(m_value); }
    };

    inline bool operator==(const Value& A, const Value& B) noexcept
    {
        // values should have the same type
        if (A.m_type != B.m_type)
            return false;
        // all the types >= Nil are Nil itself, True, False, Undefined
        if (A.typeNum() >= static_cast<uint8_t>(ValueType::Nil))
            return true;

        return A.m_value == B.m_value;
    }

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
            case ValueType::Nil:
            case ValueType::False:
                return true;

            case ValueType::True:
                return false;

            default:
                return false;
        }
    }
}

#endif
