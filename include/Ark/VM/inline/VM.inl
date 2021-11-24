// ------------------------------------------
//               instructions
// ------------------------------------------

#define resolveRef(valptr) (((valptr)->valueType() == ValueType::Reference) ? *((valptr)->reference()) : *(valptr))
#define resolveRefInPlace(val)                            \
    if (val.valueType() == ValueType::Reference)          \
    {                                                     \
        val.m_const_type = val.reference()->m_const_type; \
        val.m_value = val.reference()->m_value;           \
    }

// profiler
#include <Ark/Profiling.hpp>

template <typename... Args>
Value VM::call(const std::string& name, Args&&... args)
{
    using namespace internal;

    const std::lock_guard<std::mutex> lock(m_mutex);

    // reset ip and pp
    m_ip = 0;
    m_pp = 0;

    // find id of function
    auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), name);
    if (it == m_state->m_symbols.end())
        throwVMError("unbound variable: " + name);

    // convert and push arguments in reverse order
    std::vector<Value> fnargs { { Value(std::forward<Args>(args))... } };
    for (auto it2 = fnargs.rbegin(), it_end = fnargs.rend(); it2 != it_end; ++it2)
        push(*it2);

    // find function object and push it if it's a pageaddr/closure
    uint16_t id = static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it));
    Value* var = findNearestVariable(id);
    if (var != nullptr)
    {
        ValueType vt = var->valueType();

        if (vt != ValueType::PageAddr &&
            vt != ValueType::Closure &&
            !(vt == ValueType::Reference &&
              (var->reference()->valueType() == ValueType::PageAddr ||
               var->reference()->valueType() == ValueType::Closure)))
            throwVMError("Can't call '" + name + "': it isn't a Function but a " + types_to_str[static_cast<int>(vt)]);

        push(Value(var));
        m_last_sym_loaded = id;
    }
    else
        throwVMError("Couldn't find variable " + name);

    std::size_t frames_count = m_fc;
    // call it
    call(static_cast<int16_t>(sizeof...(Args)));
    // reset instruction pointer, otherwise the safeRun method will start at ip = -1
    // without doing m_ip++ as intended (done right after the call() in the loop, but here
    // we start outside this loop)
    m_ip = 0;

    // run until the function returns
    safeRun(/* untilFrameCount */ frames_count);

    // get result
    return *popAndResolveAsPtr();
}

template <typename... Args>
Value VM::resolve(const Value* val, Args&&... args)
{
    using namespace internal;

    const std::lock_guard<std::mutex> lock(m_mutex);

    if (!val->isFunction())
        throw TypeError("Value::resolve couldn't resolve a non-function");

    int ip = m_ip;
    std::size_t pp = m_pp;

    // convert and push arguments in reverse order
    std::vector<Value> fnargs { { Value(std::forward<Args>(args))... } };
    for (auto it = fnargs.rbegin(), it_end = fnargs.rend(); it != it_end; ++it)
        push(resolveRef(it));
    // push function
    push(resolveRef(val));

    std::size_t frames_count = m_fc;
    // call it
    call(static_cast<int16_t>(sizeof...(Args)));
    // reset instruction pointer, otherwise the safeRun method will start at ip = -1
    // without doing m_ip++ as intended (done right after the call() in the loop, but here
    // we start outside this loop)
    m_ip = 0;

    // run until the function returns
    safeRun(/* untilFrameCount */ frames_count);

    // restore VM state
    m_ip = ip;
    m_pp = pp;

    // get result
    return *popAndResolveAsPtr();
}

#pragma region "stack management"

inline uint16_t VM::readNumber()
{
    uint16_t tmp =
        (static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip]) << 8) +
        static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip + 1]);

    ++m_ip;
    return tmp;
}

inline Value* VM::pop()
{
    if (m_sp > 0)
    {
        --m_sp;
        return &(*m_stack)[m_sp];
    }
    else
        return &m_no_value;
}

inline void VM::push(const Value& value)
{
    (*m_stack)[m_sp].m_const_type = value.m_const_type;
    (*m_stack)[m_sp].m_value = value.m_value;
    ++m_sp;
}

inline void VM::push(Value&& value)
{
    (*m_stack)[m_sp].m_const_type = std::move(value.m_const_type);
    (*m_stack)[m_sp].m_value = std::move(value.m_value);
    ++m_sp;
}

inline void VM::push(Value* valptr)
{
    (*m_stack)[m_sp].m_const_type = static_cast<uint8_t>(ValueType::Reference);
    (*m_stack)[m_sp].m_value = valptr;
    ++m_sp;
}

inline Value* VM::popAndResolveAsPtr()
{
    Value* tmp = pop();
    if (tmp->valueType() == ValueType::Reference)
        return tmp->reference();
    return tmp;
}

inline void VM::swapStackForFunCall(uint16_t argc)
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
            push(Value(static_cast<PageAddr_t>(m_pp)));
            push(Value(ValueType::InstPtr, static_cast<PageAddr_t>(m_ip)));
            break;

        case 1:
            (*m_stack)[m_sp + 1] = (*m_stack)[m_sp - 1];
            resolveRefInPlace((*m_stack)[m_sp + 1]);
            (*m_stack)[m_sp - 1] = Value(static_cast<PageAddr_t>(m_pp));
            (*m_stack)[m_sp + 0] = Value(ValueType::InstPtr, static_cast<PageAddr_t>(m_ip));
            m_sp += 2;
            break;

        default:  // 2 or more elements
        {
            const int16_t first = m_sp - argc;
            // move first argument to the very end
            (*m_stack)[m_sp + 1] = (*m_stack)[first + 0];
            resolveRefInPlace((*m_stack)[m_sp + 1]);
            // move second argument right before the last one
            (*m_stack)[m_sp + 0] = (*m_stack)[first + 1];
            resolveRefInPlace((*m_stack)[m_sp + 0]);
            // move the rest, if any
            int16_t x = 2;
            const int16_t stop = ((argc % 2 == 0) ? argc : (argc - 1)) / 2;
            while (x <= stop)
            {
                //        destination          , origin
                std::swap((*m_stack)[m_sp - x + 1], (*m_stack)[first + x]);
                resolveRefInPlace((*m_stack)[m_sp - x + 1]);
                resolveRefInPlace((*m_stack)[first + x]);
                ++x;
            }
            (*m_stack)[first + 0] = Value(static_cast<PageAddr_t>(m_pp));
            (*m_stack)[first + 1] = Value(ValueType::InstPtr, static_cast<PageAddr_t>(m_ip));
            m_sp += 2;
            break;
        }
    }

    m_fc++;
    m_scope_count_to_delete.emplace_back(0);
}

#pragma endregion

inline void VM::createNewScope() noexcept
{
    m_locals.emplace_back(std::make_shared<internal::Scope>());
}

inline Value* VM::findNearestVariable(uint16_t id) noexcept
{
    for (auto it = m_locals.rbegin(), it_end = m_locals.rend(); it != it_end; ++it)
    {
        if (auto val = (**it)[id]; val != nullptr)
            return val;
    }
    return nullptr;
}

inline void VM::returnFromFuncCall()
{
    COZ_BEGIN("ark vm returnFromFuncCall");

    --m_fc;

    m_scope_count_to_delete.pop_back();
    uint8_t del_counter = m_scope_count_to_delete.back();

    // PERF high cpu cost because destroying variants cost
    m_locals.pop_back();

    while (del_counter != 0)
    {
        m_locals.pop_back();
        del_counter--;
    }

    m_scope_count_to_delete.back() = 0;

    // stop the executing if we reach the wanted frame count
    if (m_fc == m_until_frame_count)
        m_running = false;

    COZ_END("ark vm returnFromFuncCall");
}

inline void VM::call(int16_t argc_)
{
    /*
        Argument: number of arguments when calling the function
        Job: Call function from its symbol id located on top of the stack. Take the given number of
                arguments from the top of stack and give them  to the function (the first argument taken
                from the stack will be the last one of the function). The stack of the function is now composed
                of its arguments, from the first to the last one
    */
    using namespace internal;

    COZ_BEGIN("ark vm::call");

    uint16_t argc = 0;

    // handling calls from C++ code
    if (argc_ <= -1)
    {
        ++m_ip;
        argc = (static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip]) << 8) + static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip + 1]);
        ++m_ip;
    }
    else
        argc = argc_;

    Value function = *popAndResolveAsPtr();

    switch (function.valueType())
    {
        // is it a builtin function name?
        case ValueType::CProc:
        {
            // drop arguments from the stack
            std::vector<Value> args(argc);
            for (uint16_t j = 0; j < argc; ++j)
                args[argc - 1 - j] = *popAndResolveAsPtr();

            // call proc
            push(function.proc()(args, this));
            return;
        }

        // is it a user defined function?
        case ValueType::PageAddr:
        {
            PageAddr_t new_page_pointer = function.pageAddr();

            // create dedicated frame
            createNewScope();

            swapStackForFunCall(argc);

            // store "reference" to the function to speed the recursive functions
            if (m_last_sym_loaded < m_state->m_symbols.size())
                m_locals.back()->push_back(m_last_sym_loaded, function);

            m_pp = new_page_pointer;
            m_ip = -1;  // because we are doing a m_ip++ right after that
            break;
        }

        // is it a user defined closure?
        case ValueType::Closure:
        {
            Closure& c = function.refClosure();
            PageAddr_t new_page_pointer = c.pageAddr();

            // load saved scope
            m_locals.push_back(c.scope());
            // create dedicated frame
            createNewScope();
            ++m_scope_count_to_delete.back();

            swapStackForFunCall(argc);

            m_pp = new_page_pointer;
            m_ip = -1;  // because we are doing a m_ip++ right after that
            break;
        }

        default:
            throwVMError("Can't call '" + m_state->m_symbols[m_last_sym_loaded] + "': it isn't a Function but a " + types_to_str[static_cast<int>(function.valueType())]);
    }

    // checking function arity
    std::size_t index = 0,
                needed_argc = 0;

    // every argument is a MUT declaration in the bytecode
    while (m_state->m_pages[m_pp][index] == Instruction::MUT)
    {
        needed_argc += 1;
        index += 3;  // jump the argument of MUT (integer on 2 bits, big endian)
    }

    if (needed_argc != argc)
        throwVMError(
            "Function '" + m_state->m_symbols[m_last_sym_loaded] + "' needs " + std::to_string(needed_argc) +
            " arguments, but it received " + std::to_string(argc));

    COZ_END("ark vm::call");
}

#undef resolveRef
#undef resolveRefInPlace
