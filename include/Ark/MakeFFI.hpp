#if defined(FFI_VM)
    #define FFI_Function(name) Value name(const std::vector<Value>& n)
    #define FFI_Value Value
    #define FFI_isNumber(value) (value).isNumber()
    #define FFI_isString(value) (value).isString()
    #define FFI_isList(value)   (value).isList()
    #define FFI_makeList(value) FFI_Value value(true)
    #define FFI_number(value) (value).number()
    #define FFI_string(value) (value).string()
#elif defined(FFI_INTERPRETER)
    #define FFI_Function(name) Node name(const Nodes& n)
    #define FFI_Value Node
    #define FFI_isNumber(value) ((value).nodeType() == NodeType::Number)
    #define FFI_isString(value) ((value).nodeType() == NodeType::String)
    #define FFI_isList(value)   ((value).nodeType() == NodeType::List)
    #define FFI_makeList(value) FFI_Value value(NodeType::List)
    #define FFI_number(value) (value).getIntVal()
    #define FFI_string(value) (value).getStringVal()
#endif

#if defined(FFI_VM) || defined(FFI_INTERPRETER)
    #define FFI_isBool(value) ((value) == falseSym || (value) == trueSym)

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
    FFI_Function(input);  // input 0-1

    FFI_Function(toNumber);  // toNumber 1
    FFI_Function(toString);  // toString 1

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
        {
            if (!FFI_isString(n[1]))
                FFI_throwTypeError("Arguments of > should have the same type");
            
            return (FFI_string(n[0]) > FFI_string(n[1])) ? trueSym : falseSym;
        }
        else if (FFI_isNumber(n[0]))
        {
            if (!FFI_isNumber(n[1]))
                FFI_throwTypeError("Arguments of > should have the same type");
            
            return (FFI_number(n[0]) > FFI_number(n[1])) ? trueSym : falseSym;
        }
        FFI_throwTypeError("Arguments of > should either be Strings or Numbers");
    }

    FFI_Function(lt)
    {
        if (FFI_isString(n[0]))
        {
            if (!FFI_isString(n[1]))
                FFI_throwTypeError("Arguments of < should have the same type");
            
            return (FFI_string(n[0]) < FFI_string(n[1])) ? trueSym : falseSym;
        }
        else if (FFI_isNumber(n[0]))
        {
            if (!FFI_isNumber(n[1]))
                FFI_throwTypeError("Arguments of < should have the same type");
            
            return (FFI_number(n[0]) < FFI_number(n[1])) ? trueSym : falseSym;
        }
        FFI_throwTypeError("Arguments of < should either be Strings or Numbers");
    }

    FFI_Function(le)
    {
        if (FFI_isString(n[0]))
        {
            if (!FFI_isString(n[1]))
                FFI_throwTypeError("Arguments of <= should have the same type");
            
            return (FFI_string(n[0]) <= FFI_string(n[1])) ? trueSym : falseSym;
        }
        else if (FFI_isNumber(n[0]))
        {
            if (!FFI_isNumber(n[1]))
                FFI_throwTypeError("Arguments of <= should have the same type");
            
            return (FFI_number(n[0]) <= FFI_number(n[1])) ? trueSym : falseSym;
        }
        FFI_throwTypeError("Arguments of <= should either be Strings or Numbers");
    }

    FFI_Function(ge)
    {
        if (FFI_isString(n[0]))
        {
            if (!FFI_isString(n[1]))
                FFI_throwTypeError("Arguments of >= should have the same type");
            
            return (FFI_string(n[0]) >= FFI_string(n[1])) ? trueSym : falseSym;
        }
        else if (FFI_isNumber(n[0]))
        {
            if (!FFI_isNumber(n[1]))
                FFI_throwTypeError("Arguments of >= should have the same type");
            
            return (FFI_number(n[0]) >= FFI_number(n[1])) ? trueSym : falseSym;
        }
        FFI_throwTypeError("Arguments of >= should either be Strings or Numbers");
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
        if (FFI_isList(n[0]))
            return FFI_Value(static_cast<int>(n[0].const_list().size()));
        if (FFI_isString(n[0]))
            return FFI_Value(static_cast<int>(FFI_string(n[0]).size()));

        FFI_throwTypeError("Argument of len must be a list or a String");
    }
    
    FFI_Function(empty)
    {
        if (!FFI_isList(n[0]))
            FFI_throwTypeError("Argument of empty must be a list");
        
        return (n[0].const_list().size() == 0) ? trueSym : falseSym;
    }
    
    FFI_Function(firstof)
    {
        if (!FFI_isList(n[0]))
            FFI_throwTypeError("Argument of firstof must be a list");
        
        return n[0].const_list()[0];
    }
    
    FFI_Function(tailof)
    {
        if (!FFI_isList(n[0]))
            FFI_throwTypeError("Argument of tailof must be a list");
        
        if (n[0].const_list().size() < 2)
            return nil;
        
        FFI_Value r = n[0];
        r.list().erase(r.const_list().begin());
        return r;
    }

    FFI_Function(append)
    {
        if (!FFI_isList(n[0]))
            FFI_throwTypeError("First argument of append must be a list");
        
        FFI_Value r = n[0];
        for (FFI_Value::Iterator it=n.begin()+1; it != n.end(); ++it)
            r.push_back(*it);
        return r;
    }

    FFI_Function(concat)
    {
        if (!FFI_isList(n[0]))
            FFI_throwTypeError("First argument of concat should be a list");
        
        FFI_Value r = n[0];
        for (FFI_Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (!FFI_isList(*it))
                FFI_throwTypeError("Arguments of concat must be lists");

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
        if (!FFI_isBool(n[0]))
            FFI_throwTypeError("First argument of assert must be a Bool");
        
        if (n[0] == falseSym)
        {
            if (!FFI_isString(n[1]))
                FFI_throwTypeError("Second argument of assert must be a String");

            Ark::logger.error("[Assertion failed] " + FFI_string(n[1]));
            exit(1);
        }
        return nil;
    }

    FFI_Function(input)
    {
        if (n.size() == 1)
        {
            if (!FFI_isString(n[0]))
                FFI_throwTypeError("Argument of input must be of type String");
            std::cout << FFI_string(n[0]);
        }

        std::string line = "";
        std::getline(std::cin, line);

        return FFI_Value(line);
    }

    // ------------------------------

    FFI_Function(toNumber)
    {
        if (!FFI_isString(n[0]))
            FFI_throwTypeError("Argument of toNumber must be a String");
        
        return FFI_Value(Ark::BigNum(FFI_string(n[0]).c_str()));
    }

    FFI_Function(toString)
    {
        std::stringstream ss;
        ss << n[0];
        return FFI_Value(ss.str());
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
            env["input"] = Node(&input);

            env["toNumber"] = Node(&toNumber);
            env["toString"] = Node(&toString);
        }
    #endif  // FFI_INTERPRETER
#elif defined(FFI_MAKE_EXTERNS_INC)
    extern const std::vector<std::string> builtins;
#elif defined(FFI_MAKE_EXTERNS_SRC)
    extern const std::vector<std::string> builtins = {
        "+", "-", "*", "/",
        ">", "<", "<=", ">=", "!=", "=",
        "len", "empty?", "firstof", "tailof", "append", "concat", "list", "nil?",
        "print", "assert", "input",
        "toNumber", "toString"
    };
#elif defined(FFI_INIT_VM_FFI)
    m_ffi.push_back(&FFI::add);
    m_ffi.push_back(&FFI::sub);
    m_ffi.push_back(&FFI::mul);
    m_ffi.push_back(&FFI::div);

    m_ffi.push_back(&FFI::gt);
    m_ffi.push_back(&FFI::lt);
    m_ffi.push_back(&FFI::le);
    m_ffi.push_back(&FFI::ge);
    m_ffi.push_back(&FFI::neq);
    m_ffi.push_back(&FFI::eq);

    m_ffi.push_back(&FFI::len);
    m_ffi.push_back(&FFI::empty);
    m_ffi.push_back(&FFI::firstof);
    m_ffi.push_back(&FFI::tailof);
    m_ffi.push_back(&FFI::append);
    m_ffi.push_back(&FFI::concat);
    m_ffi.push_back(&FFI::list);
    m_ffi.push_back(&FFI::isnil);

    m_ffi.push_back(&FFI::print);
    m_ffi.push_back(&FFI::assert_);
    m_ffi.push_back(&FFI::input);

    m_ffi.push_back(&FFI::toNumber);
    m_ffi.push_back(&FFI::toString);
#endif  // FFI_MAKE_HEADER | FFI_MAKE_SOURCE | FFI_MAKE_EXTERNS_INC | FFI_MAKE_EXTERNS_SRC | FFI_INIT_VM_FFI
