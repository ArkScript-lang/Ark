#include <Ark/Lang/Lib.hpp>

#include <iostream>
#include <Ark/Log.hpp>

namespace Ark
{
    namespace Lang
    {
        Node add(const Nodes& n)
        {
            auto i = n[0].getIntVal();
            for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                i += it->getIntVal();
            return Node(NodeType::Number, i);
        }

        Node sub(const Nodes& n)
        {
            auto i = n[0].getIntVal();
            for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                i -= it->getIntVal();
            return Node(NodeType::Number, i);
        }

        Node mul(const Nodes& n)
        {
            auto i = n[0].getIntVal();
            for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                i *= it->getIntVal();
            return Node(NodeType::Number, i);
        }

        Node div(const Nodes& n)
        {
            auto i = n[0].getIntVal();
            for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
                i /= it->getIntVal();
            return Node(NodeType::Number, i);
        }
        
        // ------------------------------

        Node gt(const Nodes& n)
        {
            if (n[0].nodeType() == NodeType::String)
                return falseSym;
            else if (n[0].nodeType() == NodeType::Number)
            {
                auto i = n[0].getIntVal();
                return (i > n[1].getIntVal()) ? trueSym : falseSym;
            }
            return falseSym;
        }

        Node lt(const Nodes& n)
        {
            if (n[0].nodeType() == NodeType::String)
                return falseSym;
            else if (n[0].nodeType() == NodeType::Number)
            {
                auto i = n[0].getIntVal();
                return (i < n[1].getIntVal()) ? trueSym : falseSym;
            }
            return falseSym;
        }

        Node le(const Nodes& n)
        {
            if (n[0].nodeType() == NodeType::String)
                return falseSym;
            else if (n[0].nodeType() == NodeType::Number)
            {
                auto i = n[0].getIntVal();
                return (i <= n[1].getIntVal()) ? trueSym : falseSym;
            }
            return falseSym;
        }

        Node ge(const Nodes& n)
        {
            if (n[0].nodeType() == NodeType::String)
                return falseSym;
            else if (n[0].nodeType() == NodeType::Number)
            {
                auto i = n[0].getIntVal();
                return (i >= n[1].getIntVal()) ? trueSym : falseSym;
            }
            return falseSym;
        }

        Node neq(const Nodes& n)
        {
            return (!(n[0] == n[1])) ? trueSym : falseSym;
        }

        Node eq(const Nodes& n)
        {
            return (n[0] == n[1]) ? trueSym : falseSym;
        }
        
        // ------------------------------
        
        Node len(const Nodes& n)
        {
            return Node(NodeType::Number, (int) n[0].const_list().size());
        }
        
        Node empty(const Nodes& n)
        {
            return (n[0].const_list().size() == 0) ? trueSym : falseSym;
        }
        
        Node firstof(const Nodes& n)
        {
            return n[0].const_list()[0];
        }
        
        Node tailof(const Nodes& n)
        {
            if (n[0].const_list().size() < 2)
                return nil;
            
            Node r = n[0];
            r.list().erase(r.list().begin());
            return r;
        }

        Node append(const Nodes& n)
        {
            Node r = n[0];
            for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
            {
                r.push_back(*it);
            }
            return r;
        }

        Node concat(const Nodes& n)
        {
            Node r = n[0];
            for (Node::Iterator it=n.begin()+1; it != n.end(); ++it)
            {
                for (Node::Iterator it2=it->const_list().begin(); it2 != it->const_list().end(); ++it2)
                    r.push_back(*it2);
            }
            return r;
        }

        Node list(const Nodes& n)
        {
            Node r(NodeType::List);
            for (Node::Iterator it=n.begin(); it != n.end(); ++it)
            {
                r.push_back(*it);
            }
            return r;
        }

        Node isnil(const Nodes& n)
        {
            return n[0] == nil ? trueSym : falseSym;
        }
        
        // ------------------------------

        Node print(const Nodes& n)
        {
            for (Node::Iterator it=n.begin(); it != n.end(); ++it)
                std::cout << (*it) << " ";
            std::cout << std::endl;

            return nil;
        }

        Node assert_(const Nodes& n)
        {
            if (n[0] == falseSym)
            {
                Ark::logger.error("[Assertion failed] " + n[1].getStringVal());
                exit(1);
            }
            return nil;
        }
        
        // ------------------------------

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
            
            env["len"] = Node(&len);
            env["empty?"] = Node(&empty);
            env["firstof"] = Node(&firstof);
            env["tailof"] = Node(&tailof);
            env["append"] = Node(&append);
            env["concat"] = Node(&concat);
            env["list"] = Node(&list);
            env["nil?"] = Node(&isnil);

            env["print"] = Node(&print);
            env["assert"] = Node(&assert_);
        }

        extern const std::vector<std::string> builtins = {
            "+", "-", "*", "/",
            ">", "<", "<=", ">=", "!=", "=",
            "len", "empty?", "firstof", "tailof", "append", "concat", "list", "nil?",
            "print", "assert"
        };
    }
}
