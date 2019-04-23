#ifndef ark_node
#define ark_node

#include <variant>
#include <iostream>
#include <string>

#include <Ark/Parser/Definition.hpp>
#include <Ark/Parser/Set.hpp>
#include <Ark/Parser/Function.hpp>
#include <Ark/Parser/IfCond.hpp>
#include <Ark/Parser/WhileLoop.hpp>
#include <Ark/Parser/Value.hpp>
#include <Ark/Parser/Block.hpp>

namespace Ark
{
    namespace Parser
    {
        enum class NodeType
        {
            Def,
            Set,
            Fun,
            If,
            While,
            Value,
            Block
        };

        class Node
        {
        public:
            using Value_t = std::variant<Definition, Set, Function, IfCond, WhileLoop, Value, Block>;

            Node();
            ~Node();

            friend std::ostream& operator<<(std::ostream& os, const Node& N);
        
        private:
            Value_t m_value;
            NodeType m_type;
        };
    }
}

#endif  // ark_node