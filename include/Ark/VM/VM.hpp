#ifndef ark_vm
#define ark_vm

#include <vector>
#include <string>
#include <cinttypes>

#include <Ark/VM/Value.hpp>
#include <Ark/VM/Frame.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Lang/Environment.hpp>

namespace Ark
{
    namespace VM
    {
        using namespace Ark::Compiler;

        class VM
        {
        public:
            VM(bool debug=false);
            ~VM();

            void feed(const std::string& filename);
            void run();

            void loadFunction(const std::string& name, Ark::Lang::Node::ProcType function);
        
        private:
            bool m_debug;
            bytecode_t m_bytecode;
            // Instruction Pointer and Page Pointer
            std::size_t m_ip, m_pp;
            bool m_running;

            Ark::Lang::Environment m_ffi;

            std::vector<std::string> m_symbols;
            std::vector<Value> m_constants;
            std::vector<bytecode_t> m_pages;

            std::vector<Frame> m_frames;

            void configure();
            inline uint16_t readNumber()
            {
                return (static_cast<uint16_t>(m_pages[m_pp][  m_ip]) << 8) +
                       (static_cast<uint16_t>(m_pages[m_pp][++m_ip])     );
            }

            inline Ark::Lang::Node convertValueToNode(const Value& value)
            {
                if (value.isNumber())
                    return Ark::Lang::Node(value.number());
                else if (value.isString())
                    return Ark::Lang::Node(value.string());
                else if (value.isNFT())
                {
                    NFT nft = value.nft();
                    if (nft == NFT::Nil)
                        return Ark::Lang::nil;
                    else if (nft == NFT::False)
                        return Ark::Lang::falseSym;
                    else  // nft == NFT::True
                        return Ark::Lang::trueSym;
                }
                else if (value.isList())
                {
                    Ark::Lang::Node content(Ark::Lang::NodeType::List);
                    auto values = value.list();
                    for (std::size_t j=0; j < values.size(); ++j)
                        content.push_back(convertValueToNode(values[j]));
                    return content;
                }
                else if (value.isProc())
                    return Ark::Lang::Node(value.proc());
                return Ark::Lang::nil;
            }

            inline Value convertNodeToValue(const Ark::Lang::Node& node)
            {
                if (node.nodeType() == Ark::Lang::NodeType::String)
                    return Value(node.getStringVal());
                else if (node.nodeType() == Ark::Lang::NodeType::Number)
                    return Value(node.getIntVal());
                else if (node.nodeType() == Ark::Lang::NodeType::Symbol)
                {
                    if (node == Ark::Lang::nil)
                        return Value(NFT::Nil);
                    else if (node == Ark::Lang::falseSym)
                        return Value(NFT::False);
                    else if (node == Ark::Lang::trueSym)
                        return Value(NFT::True);
                }
                else if (node.nodeType() == Ark::Lang::NodeType::List)
                {
                    std::vector<Value> values;
                    for (Ark::Lang::Node::Iterator it=node.const_list().begin(); it != node.const_list().end(); ++it)
                        values.push_back(convertNodeToValue(*it));
                    return Value(values);
                }
                return Value(NFT::Nil);
            }

            Value pop();
            void push(const Value& value);

            // instructions
            void nop();
            void loadSymbol();
            void loadConst();
            void popJumpIfTrue();
            void store();
            void let();
            void popJumpIfFalse();
            void jump();
            void ret();
            void call();
            void newEnv();
            void builtin();
        };
    }
}

#endif