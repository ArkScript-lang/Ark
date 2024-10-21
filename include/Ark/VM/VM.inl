template <typename... Args>
Value VM::call(const std::string& name, Args&&... args)
{
    internal::ExecutionContext& context = *m_execution_contexts.back();

    using namespace internal;

    // reset ip and pp
    context.ip = 0;
    context.pp = 0;

    // find id of function
    const auto it = std::ranges::find(m_state.m_symbols, name);
    assert(it != m_state.m_symbols.end() && "Unbound variable");

    // convert and push arguments in reverse order
    std::vector<Value> fnargs { { Value(std::forward<Args>(args))... } };
    for (auto it2 = fnargs.rbegin(), it_end = fnargs.rend(); it2 != it_end; ++it2)
        push(*it2, context);

    // find function object and push it if it's a pageaddr/closure
    if (const auto dist = std::distance(m_state.m_symbols.begin(), it); std::cmp_less(dist, std::numeric_limits<uint16_t>::max()))
    {
        const uint16_t id = static_cast<uint16_t>(dist);
        Value* var = findNearestVariable(id, context);
        assert(var != nullptr && "Couldn't find variable");

        if (!var->isFunction())
            throwVMError(ErrorKind::Type, fmt::format("Can't call '{}': it isn't a Function but a {}", name, types_to_str[static_cast<std::size_t>(var->valueType())]));

        push(Value(var), context);
        context.last_symbol = id;
    }

    const std::size_t frames_count = context.fc;
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

    const std::size_t ip = context.ip;
    const std::size_t pp = context.pp;

    // convert and push arguments in reverse order
    std::vector<Value> fnargs { { Value(std::forward<Args>(args))... } };
    for (auto it = fnargs.rbegin(), it_end = fnargs.rend(); it != it_end; ++it)
        push(*it, context);
    // push function
    push(*val, context);

    const std::size_t frames_count = context.fc;
    // call it
    call(context, static_cast<uint16_t>(sizeof...(Args)));
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
        throw TypeError(fmt::format("VM::resolve couldn't resolve a non-function ({})", types_to_str[static_cast<std::size_t>(n[0].valueType())]));

    const std::size_t ip = context->ip;
    const std::size_t pp = context->pp;

    // convert and push arguments in reverse order
    for (auto it = n.begin() + 1, it_end = n.end(); it != it_end; ++it)
        push(*it, *context);
    push(n[0], *context);

    const std::size_t frames_count = context->fc;
    // call it
    call(*context, static_cast<uint16_t>(n.size() - 1));
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

#pragma region "instruction helpers"

inline Value* VM::loadSymbol(const uint16_t id, internal::ExecutionContext& context)
{
    context.last_symbol = id;
    if (Value* var = findNearestVariable(context.last_symbol, context); var != nullptr) [[likely]]
    {
        // push internal reference, shouldn't break anything so far, unless it's already a ref
        if (var->valueType() == ValueType::Reference)
            return var->reference();
        return var;
    }
    else [[unlikely]]
        throwVMError(internal::ErrorKind::Scope, fmt::format("Unbound variable `{}'", m_state.m_symbols[context.last_symbol]));
    return nullptr;
}

inline Value* VM::loadConstAsPtr(const uint16_t id) const
{
    return &m_state.m_constants[id];
}

inline void VM::store(const uint16_t id, const Value* val, internal::ExecutionContext& context)
{
    // avoid adding the pair (id, _) multiple times, with different values
    Value* local = context.locals.back()[id];
    if (local == nullptr) [[likely]]
        context.locals.back().push_back(id, *val);
    else
        *local = *val;
}

inline void VM::setVal(const uint16_t id, const Value* val, internal::ExecutionContext& context)
{
    if (Value* var = findNearestVariable(id, context); var != nullptr) [[likely]]
    {
        if (var->valueType() == ValueType::Reference)
            *var->reference() = *val;
        else [[likely]]
            *var = *val;
    }
    else
        throwVMError(
            internal::ErrorKind::Scope,
            fmt::format(
                "Unbound variable `{}', can not change its value to {}",
                m_state.m_symbols[id],
                val->toString(*this)));
}

#pragma endregion

#pragma region "stack management"

inline Value* VM::pop(internal::ExecutionContext& context)
{
    if (context.sp > 0) [[likely]]
    {
        --context.sp;
        return &context.stack[context.sp];
    }
    return &m_undefined_value;
}

inline void VM::push(const Value& value, internal::ExecutionContext& context)
{
    context.stack[context.sp].m_type = value.m_type;
    context.stack[context.sp].m_value = value.m_value;
    ++context.sp;
}

inline void VM::push(Value&& value, internal::ExecutionContext& context)
{
    context.stack[context.sp].m_type = std::move(value.m_type);
    context.stack[context.sp].m_value = std::move(value.m_value);
    ++context.sp;
}

inline void VM::push(Value* valptr, internal::ExecutionContext& context)
{
    context.stack[context.sp].m_type = ValueType::Reference;
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

inline void VM::swapStackForFunCall(const uint16_t argc, internal::ExecutionContext& context)
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
            const auto first = static_cast<int16_t>(context.sp - argc);
            // move first argument to the very end
            context.stack[context.sp + 1] = context.stack[static_cast<std::size_t>(first + 0)];
            //  move second argument right before the last one
            context.stack[context.sp + 0] = context.stack[static_cast<std::size_t>(first + 1)];
            //  move the rest, if any
            uint16_t x = 2;
            const uint16_t stop = ((argc % 2 == 0) ? argc : (argc - 1)) / 2;
            while (x <= stop)
            {
                // destination, origin
                std::swap(
                    context.stack[static_cast<std::size_t>(context.sp - x + 1)],
                    context.stack[static_cast<std::size_t>(first + x)]);
                ++x;
            }
            context.stack[static_cast<std::size_t>(first + 0)] = Value(static_cast<PageAddr_t>(context.pp));
            context.stack[static_cast<std::size_t>(first + 1)] = Value(ValueType::InstPtr, static_cast<PageAddr_t>(context.ip));
            context.sp += 2;
            break;
        }
    }

    context.fc++;
}

#pragma endregion

inline Value* VM::findNearestVariable(const uint16_t id, internal::ExecutionContext& context) noexcept
{
    for (auto it = context.locals.rbegin(), it_end = context.locals.rend(); it != it_end; ++it)
    {
        if (const auto val = (*it)[id]; val != nullptr)
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

inline void VM::call(internal::ExecutionContext& context, const uint16_t argc)
{
    /*
        Argument: number of arguments when calling the function
        Job: Call function from its symbol id located on top of the stack. Take the given number of
                arguments from the top of stack and give them  to the function (the first argument taken
                from the stack will be the last one of the function). The stack of the function is now composed
                of its arguments, from the first to the last one
    */
    using namespace internal;

    Value function = *popAndResolveAsPtr(context);
    context.stacked_closure_scopes.emplace_back(nullptr);

    switch (function.valueType())
    {
        // is it a builtin function name?
        case ValueType::CProc:
        {
            // drop arguments from the stack
            std::vector<Value> args;
            args.reserve(argc);

            for (uint16_t j = 0; j < argc; ++j)
            {
                // because we pull `argc` from the CALL instruction generated by the compiler,
                // we are guaranted to have `argc` values pushed on the stack ; thus we can
                // skip the `if (context.sp > 0)` check
                Value* val = &context.stack[context.sp - argc + j];
                if (val->valueType() == ValueType::Reference)
                    val = val->reference();
                args.emplace_back(*val);
            }
            context.sp -= argc;
            // call proc
            push(function.proc()(args, this), context);

            return;
        }

        // is it a user defined function?
        case ValueType::PageAddr:
        {
            const PageAddr_t new_page_pointer = function.pageAddr();

            // create dedicated frame
            context.locals.emplace_back();
            swapStackForFunCall(argc, context);

            // store "reference" to the function to speed the recursive functions
            if (context.last_symbol < m_state.m_symbols.size()) [[likely]]
                context.locals.back().push_back(context.last_symbol, function);

            context.pp = new_page_pointer;
            context.ip = 0;
            break;
        }

        // is it a user defined closure?
        case ValueType::Closure:
        {
            Closure& c = function.refClosure();
            const PageAddr_t new_page_pointer = c.pageAddr();

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
            if (context.last_symbol < m_state.m_symbols.size())
                throwVMError(
                    ErrorKind::Type,
                    fmt::format(
                        "`{}' is not a Function but a {}",
                        m_state.m_symbols[context.last_symbol], types_to_str[static_cast<std::size_t>(function.valueType())]));
            else
                throwVMError(
                    ErrorKind::Type,
                    fmt::format(
                        "{} is not a Function but a {}",
                        function.toString(*this), types_to_str[static_cast<std::size_t>(function.valueType())]));
        }
    }

    // checking function arity
    std::size_t index = 0,
                needed_argc = 0;

    // every argument is a MUT declaration in the bytecode
    while (m_state.m_pages[context.pp][index] == STORE)
    {
        needed_argc += 1;
        index += 4;  // instructions are on 4 bytes
    }

    if (needed_argc != argc) [[unlikely]]
    {
        if (context.last_symbol < m_state.m_symbols.size())
            throwVMError(
                ErrorKind::Arity,
                fmt::format(
                    "Function `{}' needs {} arguments, but it received {}",
                    m_state.m_symbols[context.last_symbol], needed_argc, argc));
        else
            throwVMError(
                ErrorKind::Arity,
                fmt::format(
                    "Function at page {} needs {} arguments, but it received {}",
                    context.pp, needed_argc, argc));
    }
}

inline void VM::callBuiltin(internal::ExecutionContext& context, const Value& builtin, const uint16_t argc)
{
    // drop arguments from the stack
    std::vector<Value> args;
    args.reserve(argc);

    for (uint16_t j = 0; j < argc; ++j)
    {
        // because we pull `argc` from the CALL instruction generated by the compiler,
        // we are guaranted to have `argc` values pushed on the stack ; thus we can
        // skip the `if (context.sp > 0)` check
        Value* val = &context.stack[context.sp - argc + j];
        if (val->valueType() == ValueType::Reference)
            val = val->reference();
        args.emplace_back(*val);
    }
    context.sp -= argc;
    // call proc
    push(builtin.proc()(args, this), context);
}
