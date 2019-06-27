#if defined(FFI_VM)
    #define FFI_isNumber(value) (value).isNumber()
    #define FFI_isString(value) (value).isString()
    #define FFI_isList(value)   (value).isList()
    #define FFI_makeList(value) FFI_Value value(true)
    #define FFI_number(value) (value).number()
    #define FFI_string(value) (value).string()
#endif

#if defined(FFI_VM) || defined(FFI_INTERPRETER)
    #define FFI_isBool(value) ((value) == falseSym || (value) == trueSym)
#endif
