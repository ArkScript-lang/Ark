#include <Ark/Lang/Lib.hpp>

#include <iostream>
#include <Ark/Log.hpp>

namespace Ark
{
    namespace Lang
    {
        Node add(const Nodes& n)
        {
            if (n[0].valueType() == ValueType::Int)
            {
                int i = n[0].getIntVal();
                for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    if (it->valueType() == ValueType::Int)
                        i += it->getIntVal();
                    else if (it->valueType() == ValueType::Float)
                        i += (int) it->getFloatVal();
                }
                return Node(NodeType::Number, i);
            }
            else if (n[0].valueType() == ValueType::Float)
            {
                float f = n[0].getFloatVal();
                for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    if (it->valueType() == ValueType::Int)
                        f += (float) it->getIntVal();
                    else if (it->valueType() == ValueType::Float)
                        f += it->getFloatVal();
                }
                return Node(NodeType::Number, f);
            }
            return nil;
        }

        Node sub(const Nodes& n)
        {
            if (n[0].valueType() == ValueType::Int)
            {
                int i = n[0].getIntVal();
                for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    if (it->valueType() == ValueType::Int)
                        i -= it->getIntVal();
                    else if (it->valueType() == ValueType::Float)
                        i -= (int) it->getFloatVal();
                }
                return Node(NodeType::Number, i);
            }
            else if (n[0].valueType() == ValueType::Float)
            {
                float f = n[0].getFloatVal();
                for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    if (it->valueType() == ValueType::Int)
                        f -= (float) it->getIntVal();
                    else if (it->valueType() == ValueType::Float)
                        f -= it->getFloatVal();
                }
                return Node(NodeType::Number, f);
            }
            return nil;
        }

        Node mul(const Nodes& n)
        {
            if (n[0].valueType() == ValueType::Int)
            {
                int i = n[0].getIntVal();
                for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    if (it->valueType() == ValueType::Int)
                        i *= it->getIntVal();
                    else if (it->valueType() == ValueType::Float)
                        i *= (int) it->getFloatVal();
                }
                return Node(NodeType::Number, i);
            }
            else if (n[0].valueType() == ValueType::Float)
            {
                float f = n[0].getFloatVal();
                for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    if (it->valueType() == ValueType::Int)
                        f *= (float) it->getIntVal();
                    else if (it->valueType() == ValueType::Float)
                        f *= it->getFloatVal();
                }
                return Node(NodeType::Number, f);
            }
            return nil;
        }

        Node div(const Nodes& n)
        {
            if (n[0].valueType() == ValueType::Int)
            {
                int i = n[0].getIntVal();
                for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    if (it->valueType() == ValueType::Int)
                        i /= it->getIntVal();
                    else if (it->valueType() == ValueType::Float)
                        i /= (int) it->getFloatVal();
                }
                return Node(NodeType::Number, i);
            }
            else if (n[0].valueType() == ValueType::Float)
            {
                float f = n[0].getFloatVal();
                for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    if (it->valueType() == ValueType::Int)
                        f /= (float) it->getIntVal();
                    else if (it->valueType() == ValueType::Float)
                        f /= it->getFloatVal();
                }
                return Node(NodeType::Number, f);
            }
            return nil;
        }

        Node gt(const Nodes& n)
        {
            CHECK_ARGUMENTS(2, n, ">")
        
            if (n[0].valueType() == ValueType::String)
                return falseSym;
            else if (n[0].valueType() == ValueType::Int)
            {
                int i = n[0].getIntVal();
                if (n[1].valueType() == ValueType::Int)
                    return (i > n[1].getIntVal()) ? trueSym : falseSym;
                else if (n[1].valueType() == ValueType::Float)
                    return (i > n[1].getFloatVal()) ? trueSym : falseSym;
            }
            else if (n[0].valueType() == ValueType::Float)
            {
                float f = n[0].getFloatVal();
                if (n[1].valueType() == ValueType::Int)
                    return (f > n[1].getIntVal()) ? trueSym : falseSym;
                else if (n[1].valueType() == ValueType::Float)
                    return (f > n[1].getFloatVal()) ? trueSym : falseSym;
            }
            return falseSym;
        }

        Node lt(const Nodes& n)
        {
           CHECK_ARGUMENTS(2, n, "<")
        
            if (n[0].valueType() == ValueType::String)
                return falseSym;
            else if (n[0].valueType() == ValueType::Int)
            {
                int i = n[0].getIntVal();
                if (n[1].valueType() == ValueType::Int)
                    return (i < n[1].getIntVal()) ? trueSym : falseSym;
                else if (n[1].valueType() == ValueType::Float)
                    return (i < n[1].getFloatVal()) ? trueSym : falseSym;
            }
            else if (n[0].valueType() == ValueType::Float)
            {
                float f = n[0].getFloatVal();
                if (n[1].valueType() == ValueType::Int)
                    return (f < n[1].getIntVal()) ? trueSym : falseSym;
                else if (n[1].valueType() == ValueType::Float)
                    return (f < n[1].getFloatVal()) ? trueSym : falseSym;
            }
            return falseSym;
        }

        Node le(const Nodes& n)
        {
            CHECK_ARGUMENTS(2, n, "<=")
        
            if (n[0].valueType() == ValueType::String)
                return falseSym;
            else if (n[0].valueType() == ValueType::Int)
            {
                int i = n[0].getIntVal();
                if (n[1].valueType() == ValueType::Int)
                    return (i <= n[1].getIntVal()) ? trueSym : falseSym;
                else if (n[1].valueType() == ValueType::Float)
                    return (i <= n[1].getFloatVal()) ? trueSym : falseSym;
            }
            else if (n[0].valueType() == ValueType::Float)
            {
                float f = n[0].getFloatVal();
                if (n[1].valueType() == ValueType::Int)
                    return (f <= n[1].getIntVal()) ? trueSym : falseSym;
                else if (n[1].valueType() == ValueType::Float)
                    return (f <= n[1].getFloatVal()) ? trueSym : falseSym;
            }
            return falseSym;
        }

        Node ge(const Nodes& n)
        {
            CHECK_ARGUMENTS(2, n, ">=")
        
            if (n[0].valueType() == ValueType::String)
                return falseSym;
            else if (n[0].valueType() == ValueType::Int)
            {
                int i = n[0].getIntVal();
                if (n[1].valueType() == ValueType::Int)
                    return (i >= n[1].getIntVal()) ? trueSym : falseSym;
                else if (n[1].valueType() == ValueType::Float)
                    return (i >= n[1].getFloatVal()) ? trueSym : falseSym;
            }
            else if (n[0].valueType() == ValueType::Float)
            {
                float f = n[0].getFloatVal();
                if (n[1].valueType() == ValueType::Int)
                    return (f >= n[1].getIntVal()) ? trueSym : falseSym;
                else if (n[1].valueType() == ValueType::Float)
                    return (f >= n[1].getFloatVal()) ? trueSym : falseSym;
            }
            return falseSym;
        }

        Node neq(const Nodes& n)
        {
            CHECK_ARGUMENTS(2, n, "!=")
        
            return (!(n[0] == n[1])) ? trueSym : falseSym;
        }

        Node eq(const Nodes& n)
        {
            CHECK_ARGUMENTS(2, n, "=")
        
            return (n[0] == n[1]) ? trueSym : falseSym;
        }

        Node print(const Nodes& n)
        {
            AT_LEAST_ARGUMENTS(1, n, "print")
        
            for (Node::Iterator it=n.begin(); it != n.end(); ++it)
                std::cout << (*it) << " ";
            std::cout << std::endl;

            return nil;
        }

        Node assert(const Nodes& n)
        {
            CHECK_ARGUMENTS(2, n, "assert")
        
            if (n[0] != trueSym)
            {
                Ark::Log::error("[Assertion failed] " + n[1].getStringVal());
                exit(1);
            }
            return nil;
        }

        void registerLib(Environment& env)
        {
            env["nil"] = nil;
            env["false"] = falseSym;
            env["true"] = trueSym;

            env["+"] = Node(&add);
            env["-"] = Node(&sub);
            env["*"] = Node(&mul);
            env["/"] = Node(&div);

            env[">"] = Node(&gt);
            env["<"] = Node(&lt);
            env["<="] = Node(&le);
            env[">="] = Node(&ge);
            env["!="] = Node(&neq);
            env["="] = Node(&eq);

            env["print"] = Node(&print);
            env["assert"] = Node(&assert);
        }
    }
}
