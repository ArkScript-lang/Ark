#ifndef ark_vm_value
#define ark_vm_value

#include <vector>
#include <variant>
#include <huge_number.hpp>
#include <string>
#include <cinttypes>
#include <iostream>

namespace Ark
{
    namespace VM
    {
        using namespace dozerg;

        enum class NFT { Nil, False, True };
        using PageAddr_t = uint16_t;

        class Value
        {
        public:
            using ProcType  = Value(*)(const std::vector<Value>&);
            using Iterator = std::vector<Value>::const_iterator;
            using ValueType = std::variant<HugeNumber, std::string, PageAddr_t, NFT, ProcType>;

            Value(bool is_list=false);
            template <typename T> Value(const T& value);
            ~Value();

            Value& operator=(const Value& value);

            bool isNumber() const;
            bool isString() const;
            bool isPageAddr() const;
            bool isNFT() const;
            bool isProc() const;
            bool isList() const;

            const HugeNumber& number() const;
            const std::string& string() const;
            const PageAddr_t pageAddr() const;
            const NFT nft() const;
            const ProcType proc() const;
            const std::vector<Value>& list() const;

            std::vector<Value>& list_ref();

            void push_back(const Value& value);

            friend std::ostream& operator<<(std::ostream& os, const Value& V);
            friend inline bool operator==(const Value& A, const Value& B);

        private:
            ValueType m_value;
            std::vector<Value> m_list;
            bool m_is_list;
        };

        inline bool operator==(const Value& A, const Value& B)
        {
            if (A.m_is_list || B.m_is_list)
                return false;
            return A.m_value == B.m_value;
        }
    }
}

#endif