#define resolveRef(valptr) (((valptr)->valueType() == ValueType::Reference) ? *((valptr)->reference()) : *(valptr))

template <typename... Args>
Value VM::call(const std::string& name, Args&&... args)
{
    internal::ExecutionContext& context = *m_execution_contexts.back();

    using namespace internal;

    // reset ip and pp
    context.ip = 0;
    context.pp = 0;

    // find id of function
    auto it = std::find(m_state.m_symbols.begin(), m_state.m_symbols.end(), name);
    if (it == m_state.m_symbols.end())
        throwVMError(ErrorKind::Scope, "Unbound variable: " + name);

    // convert and push arguments in reverse order
    std::vector<Value> fnargs { { Value(std::forward<Args>(args))... } };
    for (auto it2 = fnargs.rbegin(), it_end = fnargs.rend(); it2 != it_end; ++it2)
        push(*it2, context);

    // find function object and push it if it's a pageaddr/closure
    uint16_t id = static_cast<uint16_t>(std::distance(m_state.m_symbols.begin(), it));
    Value* var = findNearestVariable(id, context);
    if (var != nullptr)
    {
        if (!var->isFunction())
            throwVMError(ErrorKind::Type, "Can't call '" + name + "': it isn't a Function but a " + types_to_str[static_cast<std::size_t>(var->valueType())]);

        push(Value(var), context);
        context.last_symbol = id;
    }
    else
        throwVMError(ErrorKind::Scope, "Couldn't find variable " + name);

    std::size_t frames_count = context.fc;
    // call it
    call(context, static_cast<int16_t>(sizeof...(Args)));
    // reset instruction pointer, otherwise the safeRun method will start at ip = -1
    // without doing context.ip++ as intended (done right after the call() in the loop, but here
    // we start outside this loop)
    context.ip = 0;

    // run until the function returns
    safeRun(context, /* untilFrameCount */ frames_count);

    // get result
    return *popAndResolveAsPtr(context);
}

template <typename... Args>
Value VM::resolve(const Value* val, Args&&... args)
{
    using namespace internal;

    // TODO: deprecate resolve(const Value*, Args&&...) and add a resolve(ExecutionContext*, const Value*, Args&&...)
    ExecutionContext& context = *m_execution_contexts.front().get();

    if (!val->isFunction())
        throw TypeError("Value::resolve couldn't resolve a non-function");

    int ip = context.ip;
    std::size_t pp = context.pp;

    // convert and push arguments in reverse order
    std::vector<Value> fnargs { { Value(std::forward<Args>(args))... } };
    for (auto it = fnargs.rbegin(), it_end = fnargs.rend(); it != it_end; ++it)
        push(resolveRef(it), context);
    // push function
    push(resolveRef(val), context);

    std::size_t frames_count = context.fc;
    // call it
    call(context, static_cast<int16_t>(sizeof...(Args)));
    // reset instruction pointer, otherwise the safeRun method will start at ip = -1
    // without doing context.ip++ as intended (done right after the call() in the loop, but here
    // we start outside this loop)
    context.ip = 0;

    // run until the function returns
    safeRun(context, /* untilFrameCount */ frames_count);

    // restore VM state
    context.ip = ip;
    context.pp = pp;

    // get result
    return *popAndResolveAsPtr(context);
}

inline Value VM::resolve(internal::ExecutionContext* context, std::vector<Value>& n)
{
    if (!n[0].isFunction())
        throw TypeError("Value::resolve couldn't resolve a non-function (" + types_to_str[static_cast<std::size_t>(n[0].valueType())] + ")");

    int ip = context->ip;
    std::size_t pp = context->pp;

    // convert and push arguments in reverse order
    for (auto it = n.begin() + 1, it_end = n.end(); it != it_end; ++it)
        push(*it, *context);  // todo use resolveRef
    push(n[0], *context);

    std::size_t frames_count = context->fc;
    // call it
    call(*context, static_cast<int16_t>(n.size() - 1));
    // reset instruction pointer, otherwise the safeRun method will start at ip = -1
    // without doing context.ip++ as intended (done right after the call() in the loop, but here
    // we start outside this loop)
    context->ip = 0;

    // run until the function returns
    safeRun(*context, /* untilFrameCount */ frames_count);

    // restore VM state
    context->ip = ip;
    context->pp = pp;

    // get result
    return *popAndResolveAsPtr(*context);
}

#pragma region "stack management"

inline Value* VM::pop(internal::ExecutionContext& context)
{
    if (context.sp > 0)
    {
        --context.sp;
        return &context.stack[context.sp];
    }
    else
        return &m_undefined_value;
}

inline void VM::push(const Value& value, internal::ExecutionContext& context)
{
    context.stack[context.sp].m_const_type = value.m_const_type;
    context.stack[context.sp].m_value = value.m_value;
    ++context.sp;
}

inline void VM::push(Value&& value, internal::ExecutionContext& context)
{
    context.stack[context.sp].m_const_type = std::move(value.m_const_type);
    context.stack[context.sp].m_value = std::move(value.m_value);
    ++context.sp;
}

inline void VM::push(Value* valptr, internal::ExecutionContext& context)
{
    context.stack[context.sp].m_const_type = static_cast<uint8_t>(ValueType::Reference);
    context.stack[context.sp].m_value = valptr;
    ++context.sp;
}

inline Value* VM::popAndResolveAsPtr(internal::ExecutionContext& context)
{
    Value* tmp = pop(context);
    if (tmp->valueType() == ValueType::Reference)
        return tmp->reference();
    return tmp;
}

inline void VM::swapStackForFunCall(uint16_t argc, internal::ExecutionContext& context)
{
    using namespace internal;

    // move values around and invert them
    //
    // values:     1,  2, 3, _, _
    // wanted:    pp, ip, 3, 2, 1
    // positions:  0,  1, 2, 3, 4
    //
    // move values first, from position x to y, with
    //    y = argc - x + 1
    // then place pp and ip
    switch (argc)  // must be positive
    {
        case 0:
            push(Value(static_cast<PageAddr_t>(context.pp)), context);
            push(Value(ValueType::InstPtr, static_cast<PageAddr_t>(context.ip)), context);
            break;

        case 1:
            context.stack[context.sp + 1] = context.stack[context.sp - 1];
            context.stack[context.sp - 1] = Value(static_cast<PageAddr_t>(context.pp));
            context.stack[context.sp + 0] = Value(ValueType::InstPtr, static_cast<PageAddr_t>(context.ip));
            context.sp += 2;
            break;

        default:  // 2 or more elements
        {
            const int16_t first = context.sp - argc;
            // move first argument to the very end
            context.stack[context.sp + 1] = context.stack[first + 0];
            //  move second argument right before the last one
            context.stack[context.sp + 0] = context.stack[first + 1];
            //  move the rest, if any
            int16_t x = 2;
            const int16_t stop = ((argc % 2 == 0) ? argc : (argc - 1)) / 2;
            while (x <= stop)
            {
                //        destination          , origin
                std::swap(context.stack[context.sp - x + 1], context.stack[first + x]);
                ++x;
            }
            context.stack[first + 0] = Value(static_cast<PageAddr_t>(context.pp));
            context.stack[first + 1] = Value(ValueType::InstPtr, static_cast<PageAddr_t>(context.ip));
            context.sp += 2;
            break;
        }
    }

    context.fc++;
}

#pragma endregion

inline Value* VM::findNearestVariable(uint16_t id, internal::ExecutionContext& context) noexcept
{
    for (auto it = context.locals.rbegin(), it_end = context.locals.rend(); it != it_end; ++it)
    {
        if (auto val = (*it)[id]; val != nullptr)
            return val;
    }
    return nullptr;
}

inline void VM::returnFromFuncCall(internal::ExecutionContext& context)
{
    --context.fc;
    context.stacked_closure_scopes.pop_back();
    // NOTE: high cpu cost because destroying variants cost
    context.locals.pop_back();
}

inline void VM::call(internal::ExecutionContext& context, int16_t argc_)
{
    /*
        Argument: number of arguments when calling the function
        Job: Call function from its symbol id located on top of the stack. Take the given number of
                arguments from the top of stack and give them  to the function (the first argument taken
                from the stack will be the last one of the function). The stack of the function is now composed
                of its arguments, from the first to the last one
    */
    using namespace internal;

    uint16_t argc = 0;

    // handling calls from C++ code
    if (argc_ <= -1)
    {
        ++context.ip;
        argc = (static_cast<uint16_t>(m_state.m_pages[context.pp][context.ip]) << 8) + static_cast<uint16_t>(m_state.m_pages[context.pp][context.ip + 1]);
        ++context.ip;
    }
    else
        argc = argc_;

    Value function = *popAndResolveAsPtr(context);
    context.stacked_closure_scopes.emplace_back(nullptr);

    switch (function.valueType())
    {
        // is it a builtin function name?
        case ValueType::CProc:
        {
            // drop arguments from the stack
            std::vector<Value> args(argc);
            for (uint16_t j = 0; j < argc; ++j)
                args[argc - 1 - j] = *popAndResolveAsPtr(context);

            // call proc
            push(function.proc()(args, this), context);
            return;
        }

        // is it a user defined function?
        case ValueType::PageAddr:
        {
            PageAddr_t new_page_pointer = function.pageAddr();

            // create dedicated frame
            context.locals.emplace_back();
            swapStackForFunCall(argc, context);

            // store "reference" to the function to speed the recursive functions
            if (context.last_symbol < m_state.m_symbols.size())
                context.locals.back().push_back(context.last_symbol, function);

            context.pp = new_page_pointer;
            context.ip = 0;
            break;
        }

        // is it a user defined closure?
        case ValueType::Closure:
        {
            Closure& c = function.refClosure();
            PageAddr_t new_page_pointer = c.pageAddr();

            // create dedicated frame
            context.locals.emplace_back();
            // load saved scope
            c.refScope().mergeRefInto(context.locals.back());
            context.stacked_closure_scopes.back() = c.scopePtr();

            swapStackForFunCall(argc, context);

            context.pp = new_page_pointer;
            context.ip = 0;
            break;
        }

        default:
        {
            if (m_state.m_symbols.size() > 0)
                throwVMError(ErrorKind::Type, "Can't call '" + m_state.m_symbols[context.last_symbol] + "': it isn't a Function but a " + types_to_str[static_cast<std::size_t>(function.valueType())]);
            else
                throwVMError(ErrorKind::Type, "A " + types_to_str[static_cast<std::size_t>(function.valueType())] + " isn't a callable");
        }
    }

    // checking function arity
    std::size_t index = 0,
                needed_argc = 0;

    // every argument is a MUT declaration in the bytecode
    // index+1 to skip the padding
    while (m_state.m_pages[context.pp][index + 1] == Instruction::MUT)
    {
        needed_argc += 1;
        index += 4;  // instructions are on 4 bytes
    }

    if (needed_argc != argc)
        throwVMError(
            ErrorKind::Arity,
            "Function '" + m_state.m_symbols[context.last_symbol] + "' needs " + std::to_string(needed_argc) +
                " arguments, but it received " + std::to_string(argc));
}

#undef resolveRef
