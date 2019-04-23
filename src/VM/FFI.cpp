#include <Ark/VM/FFI.hpp>

#include <iostream>
#include <Ark/Log.hpp>

namespace Ark
{
    namespace VM
    {
        namespace FFI
        {
            const Value falseSym = Value(NFT::False);
            const Value trueSym  = Value(NFT::True);
            const Value nil      = Value(NFT::Nil);

            Value add(const std::vector<Value>& n)
            {
                auto i = n[0].number();
                for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
                    i += it->number();
                return Value(i);
            }

            Value sub(const std::vector<Value>& n)
            {
                auto i = n[0].number();
                for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
                    i -= it->number();
                return Value(i);
            }

            Value mul(const std::vector<Value>& n)
            {
                auto i = n[0].number();
                for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
                    i *= it->number();
                return Value(i);
            }

            Value div(const std::vector<Value>& n)
            {
                auto i = n[0].number();
                for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
                    i /= it->number();
                return Value(i);
            }
            
            // ------------------------------

            Value gt(const std::vector<Value>& n)
            {
                if (n[0].isString())
                    return falseSym;
                else if (n[0].isNumber())
                {
                    auto i = n[0].number();
                    return (i > n[1].number()) ? trueSym : falseSym;
                }
                return falseSym;
            }

            Value lt(const std::vector<Value>& n)
            {
                if (n[0].isString())
                    return falseSym;
                else if (n[0].isNumber())
                {
                    auto i = n[0].number();
                    return (i < n[1].number()) ? trueSym : falseSym;
                }
                return falseSym;
            }

            Value le(const std::vector<Value>& n)
            {
                if (n[0].isString())
                    return falseSym;
                else if (n[0].isNumber())
                {
                    auto i = n[0].number();
                    return (i <= n[1].number()) ? trueSym : falseSym;
                }
                return falseSym;
            }

            Value ge(const std::vector<Value>& n)
            {
                if (n[0].isString())
                    return falseSym;
                else if (n[0].isNumber())
                {
                    auto i = n[0].number();
                    return (i >= n[1].number()) ? trueSym : falseSym;
                }
                return falseSym;
            }

            Value neq(const std::vector<Value>& n)
            {
                return (!(n[0] == n[1])) ? trueSym : falseSym;
            }

            Value eq(const std::vector<Value>& n)
            {
                return (n[0] == n[1]) ? trueSym : falseSym;
            }
            
            // ------------------------------
            
            Value len(const std::vector<Value>& n)
            {
                return Value((int) n[0].list().size());
            }
            
            Value empty(const std::vector<Value>& n)
            {
                return (n[0].list().size() == 0) ? trueSym : falseSym;
            }
            
            Value firstof(const std::vector<Value>& n)
            {
                return n[0].list()[0];
            }
            
            Value tailof(const std::vector<Value>& n)
            {
                if (n[0].list().size() < 2)
                    return nil;
                
                Value r = n[0];
                r.list_ref().erase(r.list().begin());
                return r;
            }

            Value append(const std::vector<Value>& n)
            {
                Value r = n[0];
                for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    r.push_back(*it);
                }
                return r;
            }

            Value concat(const std::vector<Value>& n)
            {
                Value r = n[0];
                for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
                {
                    for (Value::Iterator it2=it->list().begin(); it2 != it->list().end(); ++it2)
                        r.push_back(*it2);
                }
                return r;
            }

            Value list(const std::vector<Value>& n)
            {
                Value r(/* is_list */ true);
                for (Value::Iterator it=n.begin(); it != n.end(); ++it)
                    r.push_back(*it);
                return r;
            }

            Value isnil(const std::vector<Value>& n)
            {
                return n[0] == nil ? trueSym : falseSym;
            }
            
            // ------------------------------

            Value print(const std::vector<Value>& n)
            {
                for (Value::Iterator it=n.begin(); it != n.end(); ++it)
                    std::cout << (*it) << " ";
                std::cout << std::endl;

                return nil;
            }

            Value assert_(const std::vector<Value>& n)
            {
                if (n[0] == falseSym)
                {
                    Ark::logger.error("[Assertion failed] " + n[1].string());
                    exit(1);
                }
                return nil;
            }
        }
    }
}
