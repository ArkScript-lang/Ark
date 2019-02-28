#include <Ark/Lang/Lib.hpp>

#include <iostream>

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

        Node lt(const Nodes& n)
        {
            if (n[0].valueType() == ValueType::String)
                return falseSym;
            else if (n[0].valueType() == ValueType::Int)
            {
                int i = n[0].getIntVal();
                bool b = true;

                for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    if (it->valueType() == ValueType::Int)
                    {
                        if (it->getIntVal() < i)
                        {
                            b = false;
                            break;
                        }
                    }
                    else if (it->valueType() == ValueType::Float)
                    {
                        if ((int) it->getFloatVal() < i)
                        {
                            b = false;
                            break;
                        }
                    }
                }
                return b ? trueSym : falseSym;
            }
            else if (n[0].valueType() == ValueType::Float)
            {
                float f = n[0].getFloatVal();
                bool b = true;

                for (Node::Iterator it=n.begin()+1; it < n.end(); ++it)
                {
                    if (it->valueType() == ValueType::Int)
                    {
                        if (it->getIntVal() < f)
                        {
                            b = false;
                            break;
                        }
                    }
                    else if (it->valueType() == ValueType::Float)
                    {
                        if (it->getFloatVal() < f)
                        {
                            b = false;
                            break;
                        }
                    }
                }
                return b ? trueSym : falseSym;
            }
            return falseSym;
        }

        Node neq(const Nodes& n)
        {
            bool b = true;
            Node t = n[0];
            for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
            {
                if (t == *it)
                {
                    b = false;
                    break;
                }
            }

            return b ? trueSym : falseSym;
        }

        Node eq(const Nodes& n)
        {
            bool b = true;
            Node t = n[0];
            for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
            {
                if (!(t == *it))
                {
                    b = false;
                    break;
                }
            }

            return b ? trueSym : falseSym;
        }

        Node print(const Nodes& n)
        {
            for (Node::Iterator it=n.begin(); it != n.end(); ++it)
                std::cout << (*it) << " ";
            std::cout << std::endl;

            return nil;
        }

        void registerLib(Environment& env)
        {
            env["nil"] = nil;
            env["false"] = falseSym;
            env["true"] = trueSym;

            env["+"] = Node(&add);
            env["-"] = Node(&sub);

            env["<"] = Node(&lt);
            env["!="] = Node(&neq);
            env["="] = Node(&eq);

            env["print"] = Node(&print);
        }
    }
}
