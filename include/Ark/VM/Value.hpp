/**
 * @file Value.hpp
 * @author Default value type handled by the virtual machine
 * @brief 
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef ark_vm_value
#define ark_vm_value

#include <vector>
#include <variant.hpp>
#include <string>  // for conversions
#include <cinttypes>
#include <iostream>
#include <memory>
#include <functional>
#include <utility>
#include <Ark/String.hpp>  // our string implementation
#include <string.h>  // strcmp
#include <array>

#include <Ark/VM/Types.hpp>
#include <Ark/VM/Closure.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/VM/UserType.hpp>
#include <Ark/Config.hpp>
#include <Ark/Profiling.hpp>

namespace Ark
{
    class VM;
}

namespace Ark::internal
{
    // Note from the creator: we can have at most 0b01111111 (127) different types
    // because type index is stored on the 7 right most bits of a uint8_t in the class Value.
    // Order is also important because we are doing some optimizations to check ranges
    // of types based on their integer values.
    enum class ValueType
    {
        List      = 0,
        Number    = 1,
        String    = 2,
        PageAddr  = 3,
        CProc     = 4,
        Closure   = 5,
        User      = 6,

        Nil       = 7,
        True      = 8,
        False     = 9,
        Undefined = 10,
        Reference = 11
    };

    const std::array<std::string, 12> types_to_str = {
        "List",  "Number",  "String",    "Function",
        "CProc", "Closure", "UserType",  "Nil",
        "Bool",  "Bool",    "Undefined", "Reference"
    };

    class Frame;

#ifdef ARK_PROFILER_COUNT
    extern unsigned value_creations, value_copies, value_moves;
#endif

    class ARK_API_EXPORT Value
    {
    public:
        using ProcType = Value (*) (std::vector<Value>&, Ark::VM*);  // std::function<Value (std::vector<Value>&, Ark::VM*)>
        using Iterator = std::vector<Value>::iterator;
        using ConstIterator = std::vector<Value>::const_iterator;

        using Value_t  = mpark::variant<
            double,             //  8 bytes
            String,             // 16 bytes
            PageAddr_t,         //  2 bytes
            ProcType,           //  8 bytes
            Closure,            // 24 bytes
            UserType,           // 24 bytes
            std::vector<Value>, // 24 bytes
            Value*              //  8 bytes
        >;                      // +??? bytes overhead
        //                   total 24+??? bytes

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

#ifdef ARK_PROFILER_COUNT
        Value(const Value& val) noexcept;
        Value(Value&& other) noexcept;
        Value& operator=(const Value& other) noexcept;
#endif

        /**
         * @brief Construct a new Value object as a Number
         * 
         * @param value 
         */
        explicit Value(int value) noexcept;

        /**
         * @brief Construct a new Value object as a Number
         * 
         * @param value 
         */
        explicit Value(float value) noexcept;

        /**
         * @brief Construct a new Value object as a Number
         * 
         * @param value 
         */
        explicit Value(double value) noexcept;

        /**
         * @brief Construct a new Value object as a String
         * 
         * @param value 
         */
        explicit Value(const std::string& value) noexcept;

        /**
         * @brief Construct a new Value object as a String
         * 
         * @param value 
         */
        explicit Value(const String& value) noexcept;

        /**
         * @brief Construct a new Value object as a String
         * 
         * @param value 
         */
        explicit Value(const char* value) noexcept;

        /**
         * @brief Construct a new Value object as a Function
         * 
         * @param value 
         */
        explicit Value(PageAddr_t value) noexcept;

        /**
         * @brief Construct a new Value object from a C++ function
         * 
         * @param value 
         */
        explicit Value(Value::ProcType value) noexcept;

        /**
         * @brief Construct a new Value object as a List
         * 
         * @param value 
         */
        explicit Value(std::vector<Value>&& value) noexcept;

        /**
         * @brief Construct a new Value object as a Closure
         * 
         * @param value 
         */
        explicit Value(Closure&& value) noexcept;

        /**
         * @brief Construct a new Value object as a UserType
         * 
         * @param value 
         */
        explicit Value(UserType&& value) noexcept;

        /**
         * @brief Construct a new Value object as a reference to an internal object
         * 
         * @param ref 
         */
        explicit Value(Value* ref) noexcept;

        /**
         * @brief Return the value type
         * 
         * @return ValueType 
         */
        inline ValueType valueType() const noexcept;

        /**
         * @brief Check if a function is held
         * 
         * @return true on success
         * @return false on failure
         */
        inline bool isFunction() const noexcept;

        /**
         * @brief Return the stored number
         * 
         * @return double 
         */
        inline double number() const;

        /**
         * @brief Return the stored string
         * 
         * @return const String& 
         */
        inline const String& string() const;

        /**
         * @brief Return the stored list
         * 
         * @return const std::vector<Value>& 
         */
        inline const std::vector<Value>& const_list() const;

        /**
         * @brief Return the stored user type
         * 
         * @return const UserType& 
         */
        inline const UserType& usertype() const;

        /**
         * @brief Return the stored list as a reference
         * 
         * @return std::vector<Value>& 
         */
        std::vector<Value>& list();

        /**
         * @brief Return the stored string as a reference
         * 
         * @return String& 
         */
        String& string_ref();

        /**
         * @brief Return the stored user type as a reference
         * 
         * @return UserType& 
         */
        UserType& usertype_ref();

        /**
         * @brief Return the stored internal object reference
         * 
         * @return Value* 
         */
        Value* reference() const;

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

        friend std::ostream& operator<<(std::ostream& os, const Value& V) noexcept;
        friend inline bool operator==(const Value& A, const Value& B) noexcept;
        friend inline bool operator<(const Value& A, const Value& B) noexcept;
        friend inline bool operator!(const Value& A) noexcept;

        friend class Ark::VM;
        friend class Ark::internal::Frame;

    private:
        uint8_t m_constType;  ///< First bit if for constness, right most bits are for type
        Value_t m_value;

        // private getters only for the virtual machine

        /**
         * @brief Return the page address held by the value
         * 
         * @return internal::PageAddr_t 
         */
        inline internal::PageAddr_t pageAddr() const;

        /**
         * @brief Return the C Function held by the value
         * 
         * @return const ProcType& 
         */
        inline const ProcType& proc() const;

        /**
         * @brief Return the closure held by the value
         * 
         * @return const internal::Closure& 
         */
        inline const internal::Closure& closure() const;

        /**
         * @brief Return a reference to the closure held by the value
         * 
         * @return internal::Closure& 
         */
        internal::Closure& closure_ref();

        /**
         * @brief Check if the value is const or not
         * 
         * @return true 
         * @return false 
         */
        inline const bool isConst() const noexcept;

        /**
         * @brief Set the Const object
         * 
         * @param value 
         */
        inline void setConst(bool value) noexcept;
    };

    #include "inline/Value.inl"
}

#endif