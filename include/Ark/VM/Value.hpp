#ifndef ark_vm_value
#define ark_vm_value

#include <vector>
#include <variant>
#include <string>
#include <cinttypes>
#include <iostream>
#include <memory>
#include <functional>
#include <utility>

#include <Ark/VM/Types.hpp>
#include <Ark/VM/Closure.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/VM/UserType.hpp>

namespace Ark
{
    template<bool D> class VM_t;
}

namespace Ark::internal
{
    enum class ValueType
    {
        List,
        Number,
        String,
        PageAddr,
        NFT,
        CProc,
        Closure,
        User
    };

    class Frame;

    class Value
    {
    public:
        using ProcType = std::function<Value (std::vector<Value>&)>;
        using Iterator = std::vector<Value>::const_iterator;
        using Value_t  = std::variant<double, std::string, PageAddr_t, NFT, ProcType, Closure, UserType, std::vector<Value>>;

        Value();

        Value(ValueType type);
        // Number
        Value(int value);
        Value(float value);
        Value(double value);
        // String
        Value(const std::string& value);
        Value(std::string&& value);
        // Function
        Value(PageAddr_t value);
        // Nil, False, True
        Value(NFT value);
        // C function binding
        Value(Value::ProcType value);
        // List
        Value(std::vector<Value>&& value);
        // Closure
        Value(Closure&& value);
        // UserType
        Value(UserType&& value);

        // public getters
        inline ValueType valueType() const;
        inline bool isFunction() const;
        inline double number() const;
        inline const std::string& string() const;
        inline const std::vector<Value>& const_list() const;
        inline const UserType& usertype() const;

        std::vector<Value>& list();
        std::string& string_ref();
        UserType& usertype_ref();

        // work only on lists
        void push_back(const Value& value);
        void push_back(Value&& value);

        // needed by C function bindings in modules, to resolve the value of a function call
        template <typename... Args>
        Value resolve(Args&&... args) const;

        friend std::ostream& operator<<(std::ostream& os, const Value& V);
        friend inline bool operator==(const Value& A, const Value& B);
        friend inline bool operator<(const Value& A, const Value& B);
        friend inline bool operator!(const Value& A);

        template<bool D> friend class Ark::VM_t;

    private:
        Value_t m_value;
        ValueType m_type;
        bool m_const;
        Ark::VM_t<false>* m_vmf = nullptr;
        Ark::VM_t<true>* m_vmt = nullptr;

        // private getters only for the virtual machine
        inline PageAddr_t pageAddr() const;
        inline NFT nft() const;
        inline const ProcType& proc() const;
        inline const Closure& closure() const;

        Closure& closure_ref();

        void registerVM(Ark::VM_t<false>* vm);
        void registerVM(Ark::VM_t<true>* vm);
    };

    #include "inline/Value.inl"
}

#endif