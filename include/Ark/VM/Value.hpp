#ifndef ark_vm_value
#define ark_vm_value

#include <vector>
#include <variant>
#include <huge_number.hpp>
#include <string>
#include <cinttypes>

#include <Ark/Lang/Node.hpp>

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
            using ValueType = std::variant<HugeNumber, std::string, PageAddr_t, NFT, Ark::Lang::Node::ProcType>;

            Value();
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
            const Ark::Lang::Node::ProcType proc() const;
            const std::vector<Value>& list() const;

        private:
            ValueType m_value;
            std::vector<Value> m_list;
            bool m_is_list;
        };
    }
}

#endif