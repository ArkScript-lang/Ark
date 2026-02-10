#ifndef ARK_VM_VM_INL
#define ARK_VM_VM_INL

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

    // we need to push pp then ip manually, because the PUSH_RETURN_ADDRESS instruction is not present
    push(Value(static_cast<internal::PageAddr_t>(0)), context);
    push(Value(ValueType::InstPtr, static_cast<internal::PageAddr_t>(0)), context);

    // convert and push arguments
    if (sizeof...(args) > 0)
    {
        std::vector<Value> fnargs { { Value(std::forward<Args>(args))... } };
        for (auto&& arg : fnargs | std::views::reverse)
            push(arg, context);
    }

    // find function object and push it if it's a pageaddr/closure
    if (const auto dist = std::distance(m_state.m_symbols.begin(), it); std::cmp_less(dist, MaxValue16Bits))
    {
        const uint16_t id = static_cast<uint16_t>(dist);
        Value* var = findNearestVariable(id, context);
        assert(var != nullptr && "Couldn't find variable");

        if (!var->isFunction())
            throwVMError(ErrorKind::Type, fmt::format("Can't call '{}': it isn't a Function but a {}", name, std::to_string(var->valueType())));

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

inline Value VM::resolve(internal::ExecutionContext* context, const std::vector<Value>& n)
{
    if (!n[0].isFunction())
        throw TypeError(fmt::format("VM::resolve couldn't resolve a non-function ({})", std::to_string(n[0].valueType())));

    const std::size_t ip = context->ip;
    const std::size_t pp = context->pp;

    // we need to push pp then ip manually, because the PUSH_RETURN_ADDRESS instruction is not present
    push(Value(static_cast<internal::PageAddr_t>(pp)), *context);
    push(Value(ValueType::InstPtr, static_cast<internal::PageAddr_t>(ip)), *context);

    // convert and push arguments
    for (auto&& val : std::ranges::drop_view(n, 1) | std::views::reverse)
        push(val, *context);
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

inline Value* VM::loadSymbolFromIndex(const uint16_t index, internal::ExecutionContext& context)
{
    // we need to load symbols from the end, because function calls add a reference to the current function
    // upon calling it. Which changes the index by 1, making it less clear, because it needs special
    // treatment only for function calls.
    auto& [id, value] = context.locals.back().atPosReverse(index);
    context.last_symbol = id;
    if (value.valueType() == ValueType::Reference)
        return value.reference();
    return &value;
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
        context.locals.back().pushBack(id, *val);
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

inline void VM::jump(const uint16_t address, internal::ExecutionContext& context)
{
    // instructions are on 4 bytes!
    context.ip = address * 4;
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

inline Value* VM::peekAndResolveAsPtr(internal::ExecutionContext& context)
{
    if (context.sp > 0)
    {
        Value* tmp = &context.stack[context.sp - 1];
        if (tmp->valueType() == ValueType::Reference)
            return tmp->reference();
        return tmp;
    }
    return &m_undefined_value;
}

inline void VM::push(const Value& value, internal::ExecutionContext& context) noexcept
{
    context.stack[context.sp] = value;
    ++context.sp;
}

inline void VM::push(Value&& value, internal::ExecutionContext& context) noexcept
{
    context.stack[context.sp] = std::move(value);
    ++context.sp;
}

inline void VM::push(Value* valptr, internal::ExecutionContext& context) noexcept
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

#pragma endregion

inline Value* VM::findNearestVariable(const uint16_t id, internal::ExecutionContext& context) noexcept
{
    for (auto it = context.locals.rbegin(), it_end = context.locals.rend(); it != it_end; ++it)
    {
        if (Value* val = (*it)[id]; val != nullptr)
            return val;
    }
    return nullptr;
}

inline void VM::returnFromFuncCall(internal::ExecutionContext& context)
{
    --context.fc;
    context.stacked_closure_scopes.pop_back();
    context.locals.pop_back();
}

inline void VM::call(internal::ExecutionContext& context, const uint16_t argc, Value* function_ptr, internal::PageAddr_t or_address)
{
    /*
        Argument: number of arguments when calling the function
        Job: Call function from its symbol id located on top of the stack. Take the given number of
                arguments from the top of stack and give them  to the function (the first argument taken
                from the stack will be the last one of the function). The stack of the function is now composed
                of its arguments, from the first to the last one
    */
    using namespace internal;

    if (std::cmp_greater_equal(context.sp + 2, VMStackSize)) [[unlikely]]
        throwVMError(
            ErrorKind::VM,
            fmt::format(
                "Maximum recursion depth exceeded. You could consider rewriting your function `{}' to make use of tail-call optimization.",
                m_state.m_symbols[context.last_symbol]));

    ValueType call_type;
    Value* maybe_value_ptr = nullptr;
    if (function_ptr == nullptr && or_address == 0)
        maybe_value_ptr = popAndResolveAsPtr(context);
    else if (or_address != 0)
        call_type = ValueType::PageAddr;
    else
        maybe_value_ptr = function_ptr;

    if (maybe_value_ptr != nullptr)
    {
        call_type = maybe_value_ptr->valueType();
        if (call_type == ValueType::PageAddr)
            or_address = maybe_value_ptr->pageAddr();
    }

    context.stacked_closure_scopes.emplace_back(nullptr);

    switch (call_type)
    {
        // is it a builtin function name?
        case ValueType::CProc:
        {
            callBuiltin(context, *maybe_value_ptr, argc);
            return;
        }

        // is it a user defined function?
        case ValueType::PageAddr:
        {
            const PageAddr_t new_page_pointer = or_address;

            // create dedicated scope
            context.locals.emplace_back(context.scopes_storage.data(), context.locals.back().storageEnd());
            // set up pointers (frame counter, page, instruction)
            context.fc++;
            context.pp = new_page_pointer;
            context.ip = 0;
            break;
        }

        // is it a user defined closure?
        case ValueType::Closure:
        {
            const Closure& c = maybe_value_ptr->closure();
            const PageAddr_t new_page_pointer = c.pageAddr();

            // create dedicated scope
            context.locals.emplace_back(context.scopes_storage.data(), context.locals.back().storageEnd());
            // load saved scope
            c.refScope().mergeRefInto(context.locals.back());
            context.stacked_closure_scopes.back() = c.scopePtr();

            context.fc++;
            context.pp = new_page_pointer;
            context.ip = 0;
            break;
        }

        default:
        {
            throwVMError(
                ErrorKind::Type,
                fmt::format(
                    "{} is not a Function but a {}",
                    maybe_value_ptr->toString(*this), std::to_string(call_type)));
        }
    }

    // checking function arity
    std::size_t index = 0,
                needed_argc = 0;

    // every argument is a MUT declaration in the bytecode
    while (m_state.inst(context.pp, index) == STORE ||
           m_state.inst(context.pp, index) == STORE_REF)
    {
        needed_argc += 1;
        index += 4;  // instructions are on 4 bytes
    }

    // no store? check for CALL_BUILTIN_WITHOUT_RETURN_ADDRESS
    if (index == 0 && m_state.inst(context.pp, 0) == CALL_BUILTIN_WITHOUT_RETURN_ADDRESS)
    {
        const uint8_t padding = m_state.inst(context.pp, context.ip + 1);
        const uint16_t arg = static_cast<uint16_t>((m_state.inst(context.pp, context.ip + 2) << 8) +
                                                   m_state.inst(context.pp, context.ip + 3));
        needed_argc = static_cast<uint16_t>((padding << 4) | (arg & 0xf000) >> 12);
    }

    if (std::cmp_not_equal(needed_argc, argc)) [[unlikely]]
        throwArityError(argc, needed_argc, context);
}

inline void VM::callBuiltin(internal::ExecutionContext& context, const Value& builtin, const uint16_t argc, const bool remove_return_address)
{
    using namespace Ark::literals;

    // drop arguments from the stack
    std::vector<Value> args;
    args.reserve(argc);

    for (uint16_t j = 0; j < argc; ++j)
    {
        // because we pull `argc` from the CALL instruction generated by the compiler,
        // we are guaranteed to have `argc` values pushed on the stack ; thus we can
        // skip the `if (context.sp > 0)` check
        Value* val = &context.stack[context.sp - 1 - j];
        if (val->valueType() == ValueType::Reference)
            val = val->reference();
        args.emplace_back(*val);
    }
    // +2 to skip PP/IP that were pushed by PUSH_RETURN_ADDRESS
    context.sp -= static_cast<uint16_t>(argc + (remove_return_address ? 2_u16 : 0_u16));
    // call proc
    push(builtin.proc()(args, this), context);
}

#endif
