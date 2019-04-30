#if defined(FFI_VM)
    #define FFI_Function(name) Value name(const std::vector<Value>& n)
    #define FFI_Value Value
    #define FFI_isNumber(value) (value).isNumber()
    #define FFI_isString(value) (value).isString()
    #define FFI_makeList(value) FFI_Value value(true)
    #define FFI_number(value) (value).number()
    #define FFI_string(value) (value).string()
#elif defined(FFI_INTERPRETER)
    #define FFI_Function(name) Node name(const Nodes& n)
    #define FFI_Value Node
    #define FFI_isNumber(value) ((value).nodeType() == NodeType::Number)
    #define FFI_isString(value) ((value).nodeType() == NodeType::String)
    #define FFI_makeList(value) FFI_Value value(NodeType::List)
    #define FFI_number(value) (value).getIntVal()
    #define FFI_string(value) (value).getStringVal()
#endif

#if defined(FFI_VM) || defined(FFI_INTERPRETER)
    #define FFI_throwTypeError(msg) throw Ark::TypeError(msg)
    #define FFI_throwZeroDivisionError() throw Ark::ZeroDivisionError()
#endif

#if defined(FFI_MAKE_HEADER)
    #include <Ark/Exceptions.hpp>

    FFI_Function(add);  // +
    FFI_Function(sub);  // -
    FFI_Function(mul);  // *
    FFI_Function(div);  // /

    FFI_Function(gt);  // >
    FFI_Function(lt);  // <
    FFI_Function(le);  // <=
    FFI_Function(ge);  // >=
    FFI_Function(neq);  // !=
    FFI_Function(eq);  // =

    FFI_Function(len);  // len 1
    FFI_Function(empty);  // empty? 1
    FFI_Function(firstof);  // firstof 1
    FFI_Function(tailof);  // tailof +
    FFI_Function(append);  // append +
    FFI_Function(concat);  // concat +
    FFI_Function(list);  // list +
    FFI_Function(isnil);  // nil? 1

    FFI_Function(print);  // print +
    FFI_Function(assert_);  // assert 2

    #ifdef FFI_INTERPRETER
        void registerLib(Environment& env);
    #endif  // FFI_INTERPRETER
#elif defined(FFI_MAKE_SOURCE)
    #ifdef FFI_VM
        const Value falseSym = Value(NFT::False);
        const Value trueSym  = Value(NFT::True);
        const Value nil      = Value(NFT::Nil);
    #endif  // FFI_VM

    FFI_Function(add)
    {
        if (!FFI_isNumber(n[0]))
            FFI_throwTypeError("Arguments of + should be Numbers");
        
        auto i = FFI_number(n[0]);
        for (FFI_Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (!FFI_isNumber(*it))
                FFI_throwTypeError("Arguments of + should be Numbers");
            i += FFI_number(*it);
        }
        return FFI_Value(i);
    }

    FFI_Function(sub)
    {
        if (!FFI_isNumber(n[0]))
            FFI_throwTypeError("Arguments of - should be Numbers");
        
        auto i = FFI_number(n[0]);
        for (FFI_Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (!FFI_isNumber(*it))
                FFI_throwTypeError("Arguments of - should be Numbers");
            i -= FFI_number(*it);
        }
        return FFI_Value(i);
    }

    FFI_Function(mul)
    {
        if (!FFI_isNumber(n[0]))
            FFI_throwTypeError("Arguments of * should be Numbers");
        
        auto i = FFI_number(n[0]);
        for (FFI_Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (!FFI_isNumber(*it))
                FFI_throwTypeError("Arguments of * should be Numbers");
            i *= FFI_number(*it);
        }
        return FFI_Value(i);
    }

    FFI_Function(div)
    {
        if (!FFI_isNumber(n[0]))
            FFI_throwTypeError("Arguments of / should be Numbers");
        
        auto i = FFI_number(n[0]);
        for (FFI_Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (!FFI_isNumber(*it))
                FFI_throwTypeError("Arguments of / should be Numbers");
            if (FFI_number(*it) == 0)
                FFI_throwZeroDivisionError();
            i /= FFI_number(*it);
        }
        return FFI_Value(i);
    }
    
    // ------------------------------

    FFI_Function(gt)
    {
        if (FFI_isString(n[0]))
            return falseSym;
        else if (FFI_isNumber(n[0]))
        {
            auto i = FFI_number(n[0]);
            return (i > FFI_number(n[1])) ? trueSym : falseSym;
        }
        return falseSym;
    }

    FFI_Function(lt)
    {
        if (FFI_isString(n[0]))
            return falseSym;
        else if (FFI_isNumber(n[0]))
        {
            auto i = FFI_number(n[0]);
            return (i < FFI_number(n[1])) ? trueSym : falseSym;
        }
        return falseSym;
    }

    FFI_Function(le)
    {
        if (FFI_isString(n[0]))
            return falseSym;
        else if (FFI_isNumber(n[0]))
        {
            auto i = FFI_number(n[0]);
            return (i <= FFI_number(n[1])) ? trueSym : falseSym;
        }
        return falseSym;
    }

    FFI_Function(ge)
    {
        if (FFI_isString(n[0]))
            return falseSym;
        else if (FFI_isNumber(n[0]))
        {
            auto i = FFI_number(n[0]);
            return (i >= FFI_number(n[1])) ? trueSym : falseSym;
        }
        return falseSym;
    }

    FFI_Function(neq)
    {
        return (!(n[0] == n[1])) ? trueSym : falseSym;
    }

    FFI_Function(eq)
    {
        return (n[0] == n[1]) ? trueSym : falseSym;
    }
    
    // ------------------------------
    
    FFI_Function(len)
    {
        return FFI_Value((int) n[0].const_list().size());
    }
    
    FFI_Function(empty)
    {
        return (n[0].const_list().size() == 0) ? trueSym : falseSym;
    }
    
    FFI_Function(firstof)
    {
        return n[0].const_list()[0];
    }
    
    FFI_Function(tailof)
    {
        if (n[0].const_list().size() < 2)
            return nil;
        
        FFI_Value r = n[0];
        r.list().erase(r.const_list().begin());
        return r;
    }

    FFI_Function(append)
    {
        FFI_Value r = n[0];
        for (FFI_Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            r.push_back(*it);
        }
        return r;
    }

    FFI_Function(concat)
    {
        FFI_Value r = n[0];
        for (FFI_Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            for (FFI_Value::Iterator it2=it->const_list().begin(); it2 != it->const_list().end(); ++it2)
                r.push_back(*it2);
        }
        return r;
    }

    FFI_Function(list)
    {
        FFI_makeList(r);
        for (FFI_Value::Iterator it=n.begin(); it != n.end(); ++it)
            r.push_back(*it);
        return r;
    }

    FFI_Function(isnil)
    {
        return n[0] == nil ? trueSym : falseSym;
    }
    
    // ------------------------------

    FFI_Function(print)
    {
        for (FFI_Value::Iterator it=n.begin(); it != n.end(); ++it)
            std::cout << (*it) << " ";
        std::cout << std::endl;

        return nil;
    }

    FFI_Function(assert_)
    {
        if (n[0] == falseSym)
        {
            Ark::logger.error("[Assertion failed] " + FFI_string(n[1]));
            exit(1);
        }
        return nil;
    }

    // ------------------------------

    #ifdef FFI_INTERPRETER
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
    #endif  // FFI_INTERPRETER
#elif defined(FFI_MAKE_EXTERNS_INC)
    extern const std::vector<std::string> builtins;
#elif defined(FFI_MAKE_EXTERNS_SRC)
    extern const std::vector<std::string> builtins = {
        "+", "-", "*", "/",
        ">", "<", "<=", ">=", "!=", "=",
        "len", "empty?", "firstof", "tailof", "append", "concat", "list", "nil?",
        "print", "assert"
    };
#endif  // FFI_MAKE_HEADER | FFI_MAKE_SOURCE | FFI_MAKE_EXTERNS_INC | FFI_MAKE_EXTERNS_SRC
