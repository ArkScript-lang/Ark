/**
 * @file Value.hpp
 * @author Default value type handled by the virtual machine
 * @brief
 * @version 1.0
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
#include <iostream>
#include <memory>
#include <functional>
#include <utility>
#include <array>

#include <Ark/VM/Value/Closure.hpp>
#include <Ark/VM/Value/UserType.hpp>
#include <Ark/Platform.hpp>

namespace Ark
{
    class VM;

    // Note: we can have at most 0x7f (127) different types
    //     because type index is stored on the 7 right most bits of a uint8_t in the class Value.
    //     Order is also important because we are doing some optimizations to check ranges
    //     of types based on their integer values.
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
        using ConstIterator = std::vector<Value>::const_iterator;

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
        Value(ValueType type, T&& value) noexcept :
            m_const_type(static_cast<uint8_t>(type)),
            m_value(value)
        {}

        explicit Value(int value) noexcept;
        explicit Value(float value) noexcept;
        explicit Value(double value) noexcept;
        explicit Value(const std::string& value) noexcept;
        explicit Value(const char* value) noexcept;
        explicit Value(internal::PageAddr_t value) noexcept;
        explicit Value(Value::ProcType value) noexcept;
        explicit Value(std::vector<Value>&& value) noexcept;
        explicit Value(internal::Closure&& value) noexcept;
        explicit Value(UserType&& value) noexcept;
        explicit Value(Value* ref) noexcept;

        [[nodiscard]] inline ValueType valueType() const noexcept { return static_cast<ValueType>(type_num()); }
        [[nodiscard]] inline bool isFunction() const noexcept
        {
            auto type = valueType();
            return type == ValueType::PageAddr || type == ValueType::Closure || type == ValueType::CProc ||
                (type == ValueType::Reference && reference()->isFunction());
        }

        [[nodiscard]] inline double number() const { return std::get<double>(m_value); }
        [[nodiscard]] inline const std::string& string() const { return std::get<std::string>(m_value); }
        [[nodiscard]] inline const std::vector<Value>& constList() const { return std::get<std::vector<Value>>(m_value); }
        [[nodiscard]] inline const UserType& usertype() const { return std::get<UserType>(m_value); }
        [[nodiscard]] inline std::vector<Value>& list() { return std::get<std::vector<Value>>(m_value); }
        [[nodiscard]] inline std::string& stringRef() { return std::get<std::string>(m_value); }
        [[nodiscard]] inline UserType& usertypeRef() { return std::get<UserType>(m_value); }
        [[nodiscard]] inline Value* reference() const { return std::get<Value*>(m_value); }

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

    private:
        uint8_t m_const_type;  ///< First bit if for constness, right most bits are for type
        Value_t m_value;

        [[nodiscard]] inline constexpr uint8_t type_num() const noexcept { return m_const_type & 0x7f; }

        [[nodiscard]] inline internal::PageAddr_t pageAddr() const { return std::get<internal::PageAddr_t>(m_value); }
        [[nodiscard]] inline const ProcType& proc() const { return std::get<Value::ProcType>(m_value); }
        [[nodiscard]] inline const internal::Closure& closure() const { return std::get<internal::Closure>(m_value); }
        [[nodiscard]] inline internal::Closure& refClosure() { return std::get<internal::Closure>(m_value); }

        [[nodiscard]] inline bool isConst() const noexcept { return m_const_type & (1 << 7); }
        inline void setConst(bool value) noexcept
        {
            if (value)
                m_const_type |= 1 << 7;
            else
                m_const_type = type_num();
        }
    };

#include "inline/Value.inl"
}

#endif
