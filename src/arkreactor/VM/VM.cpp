#include <Ark/VM/VM.hpp>

#include <utility>
#include <numeric>
#include <limits>
#include <ranges>
#include <fmt/core.h>
#include <fmt/color.h>

#include <Ark/Files.hpp>
#include <Ark/Utils.hpp>
#include <Ark/TypeChecker.hpp>
#include <Ark/Compiler/Instructions.hpp>

struct mapping
{
    char* name;
    Ark::Value (*value)(std::vector<Ark::Value>&, Ark::VM*);
};

namespace Ark
{
    using namespace internal;

    VM::VM(State& state) noexcept :
        m_state(state), m_exit_code(0), m_running(false)
    {
        m_execution_contexts.emplace_back(std::make_unique<ExecutionContext>())->locals.reserve(4);
    }

    void VM::init() noexcept
    {
        ExecutionContext& context = *m_execution_contexts.back();
        for (const auto& c : m_execution_contexts)
        {
            c->ip = 0;
            c->pp = 0;
            c->sp = 0;
        }

        context.sp = 0;
        context.fc = 1;

        m_shared_lib_objects.clear();
        context.stacked_closure_scopes.clear();
        context.stacked_closure_scopes.emplace_back(nullptr);

        context.saved_scope.reset();
        m_exit_code = 0;

        context.locals.clear();
        context.locals.emplace_back();

        // loading bound stuff
        // put them in the global frame if we can, aka the first one
        for (const auto& [sym_id, value] : m_state.m_binded)
        {
            auto it = std::ranges::find(m_state.m_symbols, sym_id);
            if (it != m_state.m_symbols.end())
                context.locals[0].push_back(static_cast<uint16_t>(std::distance(m_state.m_symbols.begin(), it)), value);
        }
    }

    Value& VM::operator[](const std::string& name) noexcept
    {
        // find id of object
        const auto it = std::ranges::find(m_state.m_symbols, name);
        if (it == m_state.m_symbols.end())
        {
            m_no_value = Builtins::nil;
            return m_no_value;
        }

        const auto dist = std::distance(m_state.m_symbols.begin(), it);
        if (std::cmp_less(dist, std::numeric_limits<uint16_t>::max()))
        {
            ExecutionContext& context = *m_execution_contexts.front();

            const auto id = static_cast<uint16_t>(dist);
            Value* var = findNearestVariable(id, context);
            if (var != nullptr)
                return *var;
        }

        m_no_value = Builtins::nil;
        return m_no_value;
    }

    void VM::loadPlugin(const uint16_t id, ExecutionContext& context)
    {
        namespace fs = std::filesystem;

        const std::string file = m_state.m_constants[id].stringRef();

        std::string path = file;
        // bytecode loaded from file
        if (m_state.m_filename != ARK_NO_NAME_FILE)
            path = (fs::path(m_state.m_filename).parent_path() / fs::path(file)).relative_path().string();

        std::shared_ptr<SharedLibrary> lib;
        // if it exists alongside the .arkc file
        if (Utils::fileExists(path))
            lib = std::make_shared<SharedLibrary>(path);
        else
        {
            for (auto const& v : m_state.m_libenv)
            {
                std::string lib_path = (fs::path(v) / fs::path(file)).string();

                // if it's already loaded don't do anything
                if (std::ranges::find_if(m_shared_lib_objects, [&](const auto& val) {
                        return (val->path() == path || val->path() == lib_path);
                    }) != m_shared_lib_objects.end())
                    return;

                // check in lib_path
                if (Utils::fileExists(lib_path))
                {
                    lib = std::make_shared<SharedLibrary>(lib_path);
                    break;
                }
            }
        }

        if (!lib)
        {
            auto lib_path = std::accumulate(
                std::next(m_state.m_libenv.begin()),
                m_state.m_libenv.end(),
                m_state.m_libenv[0].string(),
                [](const std::string& a, const fs::path& b) -> std::string {
                    return a + "\n\t- " + b.string();
                });
            throwVMError(
                ErrorKind::Module,
                fmt::format("Could not find module '{}'. Searched under\n\t- {}\n\t- {}", file, path, lib_path));
        }

        m_shared_lib_objects.emplace_back(lib);

        // load the mapping from the dynamic library
        try
        {
            const mapping* map = m_shared_lib_objects.back()->get<mapping* (*)()>("getFunctionsMapping")();
            // load the mapping data
            std::size_t i = 0;
            while (map[i].name != nullptr)
            {
                // put it in the global frame, aka the first one
                auto it = std::ranges::find(m_state.m_symbols, std::string(map[i].name));
                if (it != m_state.m_symbols.end())
                    context.locals[0].push_back(static_cast<uint16_t>(std::distance(m_state.m_symbols.begin(), it)), Value(map[i].value));

                ++i;
            }
        }
        catch (const std::system_error& e)
        {
            throwVMError(
                ErrorKind::Module,
                fmt::format(
                    "An error occurred while loading module '{}': {}\nIt is most likely because the versions of the module and the language don't match.",
                    file, e.what()));
        }
    }

    void VM::exit(const int code) noexcept
    {
        m_exit_code = code;
        m_running = false;
    }

    ExecutionContext* VM::createAndGetContext()
    {
        const std::lock_guard lock(m_mutex);

        m_execution_contexts.push_back(std::make_unique<ExecutionContext>());
        ExecutionContext* ctx = m_execution_contexts.back().get();
        ctx->stacked_closure_scopes.emplace_back(nullptr);

        ctx->locals.reserve(m_execution_contexts.front()->locals.size());
        for (const auto& local : m_execution_contexts.front()->locals)
            ctx->locals.push_back(local);

        return ctx;
    }

    void VM::deleteContext(ExecutionContext* ec)
    {
        const std::lock_guard lock(m_mutex);

        const auto it =
            std::ranges::remove_if(
                m_execution_contexts,
                [ec](const std::unique_ptr<ExecutionContext>& ctx) {
                    return ctx.get() == ec;
                })
                .begin();
        m_execution_contexts.erase(it);
    }

    Future* VM::createFuture(std::vector<Value>& args)
    {
        ExecutionContext* ctx = createAndGetContext();

        // doing this after having created the context
        // because the context uses the mutex and we don't want a deadlock
        const std::lock_guard lock(m_mutex);
        m_futures.push_back(std::make_unique<Future>(ctx, this, args));

        return m_futures.back().get();
    }

    void VM::deleteFuture(Future* f)
    {
        const std::lock_guard lock(m_mutex);

        const auto it =
            std::ranges::remove_if(
                m_futures,
                [f](const std::unique_ptr<Future>& future) {
                    return future.get() == f;
                })
                .begin();
        m_futures.erase(it);
    }

    bool VM::forceReloadPlugins() const
    {
        // load the mapping from the dynamic library
        try
        {
            for (const auto& shared_lib : m_shared_lib_objects)
            {
                const mapping* map = shared_lib->template get<mapping* (*)()>("getFunctionsMapping")();
                // load the mapping data
                std::size_t i = 0;
                while (map[i].name != nullptr)
                {
                    // put it in the global frame, aka the first one
                    auto it = std::ranges::find(m_state.m_symbols, std::string(map[i].name));
                    if (it != m_state.m_symbols.end())
                        m_execution_contexts[0]->locals[0].push_back(
                            static_cast<uint16_t>(std::distance(m_state.m_symbols.begin(), it)),
                            Value(map[i].value));

                    ++i;
                }
            }

            return true;
        }
        catch (const std::system_error&)
        {
            return false;
        }
    }

    int VM::run(const bool fail_with_exception)
    {
        init();
        safeRun(*m_execution_contexts[0], 0, fail_with_exception);
        return m_exit_code;
    }

    int VM::safeRun(ExecutionContext& context, std::size_t untilFrameCount, bool fail_with_exception)
    {
#if ARK_USE_COMPUTED_GOTOS
#    define TARGET(op) TARGET_##op:
#    define DISPATCH_GOTO()            \
        _Pragma("GCC diagnostic push") \
            _Pragma("GCC diagnostic ignored \"-Wpedantic\"") goto* opcode_targets[inst];
        _Pragma("GCC diagnostic pop")
#    define GOTO_HALT() goto dispatch_end
#else
#    define TARGET(op) case op:
#    define DISPATCH_GOTO() goto dispatch_opcode
#    define GOTO_HALT() break
#endif

#define NEXTOPARG()                                                                      \
    do                                                                                   \
    {                                                                                    \
        inst = m_state.m_pages[context.pp][context.ip];                                  \
        padding = m_state.m_pages[context.pp][context.ip + 1];                           \
        arg = static_cast<uint16_t>((m_state.m_pages[context.pp][context.ip + 2] << 8) + \
                                    m_state.m_pages[context.pp][context.ip + 3]);        \
        context.ip += 4;                                                                 \
    } while (false)
#define DISPATCH() \
    NEXTOPARG();   \
    DISPATCH_GOTO();

#if ARK_USE_COMPUTED_GOTOS
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
            const std::array opcode_targets = {
                &&TARGET_NOP,
                &&TARGET_LOAD_SYMBOL,
                &&TARGET_LOAD_CONST,
                &&TARGET_POP_JUMP_IF_TRUE,
                &&TARGET_STORE,
                &&TARGET_SET_VAL,
                &&TARGET_POP_JUMP_IF_FALSE,
                &&TARGET_JUMP,
                &&TARGET_RET,
                &&TARGET_HALT,
                &&TARGET_CALL,
                &&TARGET_CAPTURE,
                &&TARGET_BUILTIN,
                &&TARGET_DEL,
                &&TARGET_MAKE_CLOSURE,
                &&TARGET_GET_FIELD,
                &&TARGET_PLUGIN,
                &&TARGET_LIST,
                &&TARGET_APPEND,
                &&TARGET_CONCAT,
                &&TARGET_APPEND_IN_PLACE,
                &&TARGET_CONCAT_IN_PLACE,
                &&TARGET_POP_LIST,
                &&TARGET_POP_LIST_IN_PLACE,
                &&TARGET_POP,
                &&TARGET_DUP,
                &&TARGET_ADD,
                &&TARGET_SUB,
                &&TARGET_MUL,
                &&TARGET_DIV,
                &&TARGET_GT,
                &&TARGET_LT,
                &&TARGET_LE,
                &&TARGET_GE,
                &&TARGET_NEQ,
                &&TARGET_EQ,
                &&TARGET_LEN,
                &&TARGET_EMPTY,
                &&TARGET_TAIL,
                &&TARGET_HEAD,
                &&TARGET_ISNIL,
                &&TARGET_ASSERT,
                &&TARGET_TO_NUM,
                &&TARGET_TO_STR,
                &&TARGET_AT,
                &&TARGET_MOD,
                &&TARGET_TYPE,
                &&TARGET_HASFIELD,
                &&TARGET_NOT,
            };
#    pragma GCC diagnostic pop
#endif

        try
        {
            [[maybe_unused]] uint8_t padding = 0;
            uint8_t inst = 0;
            uint16_t arg = 0;
            m_running = true;

            DISPATCH();
            {
#if !ARK_USE_COMPUTED_GOTOS
            dispatch_opcode:
                switch (inst)
#endif
                {
#pragma region "Instructions"
                    TARGET(NOP)
                    {
                        DISPATCH();
                    }

                    TARGET(LOAD_SYMBOL)
                    {
                        context.last_symbol = arg;
                        if (Value* var = findNearestVariable(context.last_symbol, context); var != nullptr) [[likely]]
                        {
                            // push internal reference, shouldn't break anything so far, unless it's already a ref
                            if (var->valueType() == ValueType::Reference)
                                push(var->reference(), context);
                            else
                                push(var, context);
                        }
                        else [[unlikely]]
                            throwVMError(ErrorKind::Scope, fmt::format("Unbound variable `{}'", m_state.m_symbols[context.last_symbol]));
                        DISPATCH();
                    }

                    TARGET(LOAD_CONST)
                    {
                        push(&(m_state.m_constants[arg]), context);
                        DISPATCH();
                    }

                    TARGET(POP_JUMP_IF_TRUE)
                    {
                        if (Value boolean = *popAndResolveAsPtr(context); !!boolean)
                            context.ip = arg * 4;  // instructions are 4 bytes
                        DISPATCH();
                    }

                    TARGET(STORE)
                    {
                        {
                            Value val = *popAndResolveAsPtr(context);
                            // avoid adding the pair (id, _) multiple times, with different values
                            Value* local = context.locals.back()[arg];
                            if (local == nullptr) [[likely]]
                                context.locals.back().push_back(arg, val);
                            else
                                *local = val;
                        }

                        DISPATCH();
                    }

                    TARGET(SET_VAL)
                    {
                        {
                            Value val = *popAndResolveAsPtr(context);
                            if (Value* var = findNearestVariable(arg, context); var != nullptr) [[likely]]
                            {
                                if (var->valueType() == ValueType::Reference)
                                    *var->reference() = val;
                                else [[likely]]
                                    *var = val;
                            }
                            else
                                throwVMError(ErrorKind::Scope, fmt::format("Unbound variable `{}', can not change its value to {}", m_state.m_symbols[arg], val.toString(*this)));
                        }
                        DISPATCH();
                    }

                    TARGET(POP_JUMP_IF_FALSE)
                    {
                        if (Value boolean = *popAndResolveAsPtr(context); !boolean)
                            context.ip = arg * 4;  // instructions are 4 bytes
                        DISPATCH();
                    }

                    TARGET(JUMP)
                    {
                        context.ip = arg * 4;  // instructions are 4 bytes
                        DISPATCH();
                    }

                    TARGET(RET)
                    {
                        {
                            Value ip_or_val = *popAndResolveAsPtr(context);
                            // no return value on the stack
                            if (ip_or_val.valueType() == ValueType::InstPtr) [[unlikely]]
                            {
                                context.ip = ip_or_val.pageAddr();
                                // we always push PP then IP, thus the next value
                                // MUST be the page pointer
                                context.pp = pop(context)->pageAddr();

                                returnFromFuncCall(context);
                                push(Builtins::nil, context);
                            }
                            // value on the stack
                            else [[likely]]
                            {
                                Value* ip;
                                do
                                {
                                    ip = popAndResolveAsPtr(context);
                                } while (ip->valueType() != ValueType::InstPtr);

                                context.ip = ip->pageAddr();
                                context.pp = pop(context)->pageAddr();

                                returnFromFuncCall(context);
                                push(std::move(ip_or_val), context);
                            }

                            if (context.fc <= untilFrameCount)
                                GOTO_HALT();
                        }

                        DISPATCH();
                    }

                    TARGET(HALT)
                    {
                        m_running = false;
                        GOTO_HALT();
                    }

                    TARGET(CALL)
                    {
                        // stack pointer + 2 because we push IP and PP
                        if (context.sp + 2u >= VMStackSize) [[unlikely]]
                            throwVMError(
                                ErrorKind::VM,
                                fmt::format(
                                    "Maximum recursion depth exceeded. You could consider rewriting your function `{}' to make use of tail-call optimization.",
                                    m_state.m_symbols[context.last_symbol]));
                        call(context, arg);
                        if (!m_running)
                            GOTO_HALT();
                        DISPATCH();
                    }

                    TARGET(CAPTURE)
                    {
                        if (!context.saved_scope)
                            context.saved_scope = Scope();

                        Value* ptr = (context.locals.back())[arg];
                        if (!ptr)
                            throwVMError(ErrorKind::Scope, fmt::format("Couldn't capture `{}' as it is currently unbound", m_state.m_symbols[arg]));
                        else
                        {
                            ptr = ptr->valueType() == ValueType::Reference ? ptr->reference() : ptr;
                            context.saved_scope.value().push_back(arg, *ptr);
                        }

                        DISPATCH();
                    }

                    TARGET(BUILTIN)
                    {
                        push(Builtins::builtins[arg].second, context);
                        DISPATCH();
                    }

                    TARGET(DEL)
                    {
                        if (Value* var = findNearestVariable(arg, context); var != nullptr)
                        {
                            if (var->valueType() == ValueType::User)
                                var->usertypeRef().del();
                            *var = Value();
                            DISPATCH();
                        }

                        throwVMError(ErrorKind::Scope, fmt::format("Can not delete unbound variable `{}'", m_state.m_symbols[arg]));
                        DISPATCH();
                    }

                    TARGET(MAKE_CLOSURE)
                    {
                        push(Value(Closure(context.saved_scope.value(), m_state.m_constants[arg].pageAddr())), context);
                        context.saved_scope.reset();
                        DISPATCH();
                    }

                    TARGET(GET_FIELD)
                    {
                        Value* var = popAndResolveAsPtr(context);
                        if (var->valueType() != ValueType::Closure)
                        {
                            if (context.last_symbol < m_state.m_symbols.size()) [[likely]]
                                throwVMError(
                                    ErrorKind::Type,
                                    fmt::format(
                                        "`{}' is a {}, not a Closure, can not get the field `{}' from it",
                                        m_state.m_symbols[context.last_symbol],
                                        types_to_str[static_cast<std::size_t>(var->valueType())],
                                        m_state.m_symbols[arg]));
                            else
                                throwVMError(ErrorKind::Type,
                                             fmt::format(
                                                 "{} is not a Closure, can not get the field `{}' from it",
                                                 types_to_str[static_cast<std::size_t>(var->valueType())],
                                                 m_state.m_symbols[arg]));
                        }

                        if (Value* field = var->refClosure().refScope()[arg]; field != nullptr)
                        {
                            // check for CALL instruction (the instruction because context.ip is already on the next instruction word)
                            if (m_state.m_pages[context.pp][context.ip] == CALL)
                                push(Value(Closure(var->refClosure().scopePtr(), field->pageAddr())), context);
                            else
                                push(field, context);
                        }
                        else
                            throwVMError(ErrorKind::Scope, fmt::format("`{}' isn't in the closure environment: {}", m_state.m_symbols[arg], var->refClosure().toString(*this)));
                        DISPATCH();
                    }

                    TARGET(PLUGIN)
                    {
                        loadPlugin(arg, context);
                        DISPATCH();
                    }

                    TARGET(LIST)
                    {
                        {
                            Value l(ValueType::List);
                            if (arg != 0)
                                l.list().reserve(arg);

                            for (uint16_t i = 0; i < arg; ++i)
                                l.push_back(*popAndResolveAsPtr(context));
                            push(std::move(l), context);
                        }
                        DISPATCH();
                    }

                    TARGET(APPEND)
                    {
                        {
                            Value* list = popAndResolveAsPtr(context);
                            if (list->valueType() != ValueType::List)
                                types::generateError(
                                    "append",
                                    { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                                    { *list });

                            const auto size = static_cast<uint16_t>(list->constList().size());

                            Value obj { *list };
                            obj.list().reserve(size + arg);

                            for (uint16_t i = 0; i < arg; ++i)
                                obj.push_back(*popAndResolveAsPtr(context));
                            push(std::move(obj), context);
                        }
                        DISPATCH();
                    }

                    TARGET(CONCAT)
                    {
                        {
                            Value* list = popAndResolveAsPtr(context);
                            if (list->valueType() != ValueType::List)
                                types::generateError(
                                    "concat",
                                    { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                                    { *list });

                            Value obj { *list };

                            for (uint16_t i = 0; i < arg; ++i)
                            {
                                Value* next = popAndResolveAsPtr(context);

                                if (list->valueType() != ValueType::List || next->valueType() != ValueType::List)
                                    types::generateError(
                                        "concat",
                                        { { types::Contract { { types::Typedef("dst", ValueType::List), types::Typedef("src", ValueType::List) } } } },
                                        { *list, *next });

                                std::ranges::copy(next->list(), std::back_inserter(obj.list()));
                            }
                            push(std::move(obj), context);
                        }
                        DISPATCH();
                    }

                    TARGET(APPEND_IN_PLACE)
                    {
                        Value* list = popAndResolveAsPtr(context);

                        if (list->valueType() != ValueType::List)
                            types::generateError(
                                "append!",
                                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                                { *list });

                        for (uint16_t i = 0; i < arg; ++i)
                            list->push_back(*popAndResolveAsPtr(context));
                        DISPATCH();
                    }

                    TARGET(CONCAT_IN_PLACE)
                    {
                        Value* list = popAndResolveAsPtr(context);

                        if (list->valueType() != ValueType::List)
                            types::generateError(
                                "concat",
                                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                                { *list });

                        for (uint16_t i = 0; i < arg; ++i)
                        {
                            Value* next = popAndResolveAsPtr(context);

                            if (list->valueType() != ValueType::List || next->valueType() != ValueType::List)
                                types::generateError(
                                    "concat!",
                                    { { types::Contract { { types::Typedef("dst", ValueType::List), types::Typedef("src", ValueType::List) } } } },
                                    { *list, *next });

                            std::ranges::copy(next->list(), std::back_inserter(list->list()));
                        }
                        DISPATCH();
                    }

                    TARGET(POP_LIST)
                    {
                        {
                            Value list = *popAndResolveAsPtr(context);
                            Value number = *popAndResolveAsPtr(context);

                            if (list.valueType() != ValueType::List || number.valueType() != ValueType::Number)
                                types::generateError(
                                    "pop",
                                    { { types::Contract { { types::Typedef("list", ValueType::List), types::Typedef("index", ValueType::Number) } } } },
                                    { list, number });

                            long idx = static_cast<long>(number.number());
                            idx = idx < 0 ? static_cast<long>(list.list().size()) + idx : idx;
                            if (std::cmp_greater_equal(idx, list.list().size()))
                                throwVMError(
                                    ErrorKind::Index,
                                    fmt::format("pop index ({}) out of range (list size: {})", idx, list.list().size()));

                            list.list().erase(list.list().begin() + idx);
                            push(list, context);
                        }
                        DISPATCH();
                    }

                    TARGET(POP_LIST_IN_PLACE)
                    {
                        {
                            Value* list = popAndResolveAsPtr(context);
                            Value number = *popAndResolveAsPtr(context);

                            if (list->valueType() != ValueType::List || number.valueType() != ValueType::Number)
                                types::generateError(
                                    "pop!",
                                    { { types::Contract { { types::Typedef("list", ValueType::List), types::Typedef("index", ValueType::Number) } } } },
                                    { *list, number });

                            long idx = static_cast<long>(number.number());
                            idx = idx < 0 ? static_cast<long>(list->list().size()) + idx : idx;
                            if (std::cmp_greater_equal(idx, list->list().size()))
                                throwVMError(
                                    ErrorKind::Index,
                                    fmt::format("pop! index ({}) out of range (list size: {})", idx, list->list().size()));

                            list->list().erase(list->list().begin() + idx);
                        }
                        DISPATCH();
                    }

                    TARGET(POP)
                    {
                        pop(context);
                        DISPATCH();
                    }

                    TARGET(DUP)
                    {
                        context.stack[context.sp] = context.stack[context.sp - 1];
                        ++context.sp;
                        DISPATCH();
                    }

#pragma endregion

#pragma region "Operators"

                    TARGET(ADD)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (a->valueType() == ValueType::Number && b->valueType() == ValueType::Number)
                            push(Value(a->number() + b->number()), context);
                        else if (a->valueType() == ValueType::String && b->valueType() == ValueType::String)
                            push(Value(a->string() + b->string()), context);
                        else
                            types::generateError(
                                "+",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } },
                                    types::Contract { { types::Typedef("a", ValueType::String), types::Typedef("b", ValueType::String) } } } },
                                { *a, *b });
                        DISPATCH();
                    }

                    TARGET(SUB)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            types::generateError(
                                "-",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });
                        push(Value(a->number() - b->number()), context);
                        DISPATCH();
                    }

                    TARGET(MUL)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            types::generateError(
                                "*",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });
                        push(Value(a->number() * b->number()), context);
                        DISPATCH();
                    }

                    TARGET(DIV)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            types::generateError(
                                "/",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });
                        auto d = b->number();
                        if (d == 0)
                            throwVMError(ErrorKind::DivisionByZero, fmt::format("Can not compute expression (/ {} {})", a->toString(*this), b->toString(*this)));

                        push(Value(a->number() / d), context);
                        DISPATCH();
                    }

                    TARGET(GT)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push((*a != *b && !(*a < *b)) ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(LT)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push((*a < *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(LE)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push((((*a < *b) || (*a == *b)) ? Builtins::trueSym : Builtins::falseSym), context);
                        DISPATCH();
                    }

                    TARGET(GE)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push(!(*a < *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(NEQ)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push((*a != *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(EQ)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push((*a == *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(LEN)
                    {
                        Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() == ValueType::List)
                            push(Value(static_cast<int>(a->constList().size())), context);
                        else if (a->valueType() == ValueType::String)
                            push(Value(static_cast<int>(a->string().size())), context);
                        else
                            types::generateError(
                                "len",
                                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                { *a });
                        DISPATCH();
                    }

                    TARGET(EMPTY)
                    {
                        Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() == ValueType::List)
                            push(a->constList().empty() ? Builtins::trueSym : Builtins::falseSym, context);
                        else if (a->valueType() == ValueType::String)
                            push(a->string().empty() ? Builtins::trueSym : Builtins::falseSym, context);
                        else
                            types::generateError(
                                "empty?",
                                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                { *a });
                        DISPATCH();
                    }

                    TARGET(TAIL)
                    {
                        Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() == ValueType::List)
                        {
                            if (a->constList().size() < 2)
                                push(Value(ValueType::List), context);
                            else
                            {
                                std::vector<Value> tmp(a->constList().size() - 1);
                                for (std::size_t i = 1, end = a->constList().size(); i < end; ++i)
                                    tmp[i - 1] = a->constList()[i];
                                push(Value(std::move(tmp)), context);
                            }
                        }
                        else if (a->valueType() == ValueType::String)
                        {
                            if (a->string().size() < 2)
                                push(Value(ValueType::String), context);
                            else
                            {
                                Value b { *a };
                                b.stringRef().erase(b.stringRef().begin());
                                push(std::move(b), context);
                            }
                        }
                        else
                            types::generateError(
                                "tail",
                                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                { *a });
                        DISPATCH();
                    }

                    TARGET(HEAD)
                    {
                        Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() == ValueType::List)
                        {
                            if (a->constList().empty())
                                push(Builtins::nil, context);
                            else
                                push(a->constList()[0], context);
                        }
                        else if (a->valueType() == ValueType::String)
                        {
                            if (a->string().empty())
                                push(Value(ValueType::String), context);
                            else
                                push(Value(std::string(1, a->stringRef()[0])), context);
                        }
                        else
                            types::generateError(
                                "head",
                                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                { *a });
                        DISPATCH();
                    }

                    TARGET(ISNIL)
                    {
                        Value* a = popAndResolveAsPtr(context);
                        push((*a == Builtins::nil) ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(ASSERT)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (b->valueType() != ValueType::String)
                            types::generateError(
                                "assert",
                                { { types::Contract { { types::Typedef("expr", ValueType::Any), types::Typedef("message", ValueType::String) } } } },
                                { *a, *b });

                        if (*a == Builtins::falseSym)
                            throw AssertionFailed(b->stringRef());
                        DISPATCH();
                    }

                    TARGET(TO_NUM)
                    {
                        Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::String)
                            types::generateError(
                                "toNumber",
                                { { types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                { *a });

                        double val;
                        if (Utils::isDouble(a->string(), &val))
                            push(Value(val), context);
                        else
                            push(Builtins::nil, context);
                        DISPATCH();
                    }

                    TARGET(TO_STR)
                    {
                        Value* a = popAndResolveAsPtr(context);
                        push(Value(a->toString(*this)), context);
                        DISPATCH();
                    }

                    TARGET(AT)
                    {
                        {
                            Value* b = popAndResolveAsPtr(context);
                            Value a = *popAndResolveAsPtr(context);  // be careful, it's not a pointer

                            if (b->valueType() != ValueType::Number)
                                types::generateError(
                                    "@",
                                    { { types::Contract { { types::Typedef("src", ValueType::List), types::Typedef("idx", ValueType::Number) } },
                                        types::Contract { { types::Typedef("src", ValueType::String), types::Typedef("idx", ValueType::Number) } } } },
                                    { a, *b });

                            long idx = static_cast<long>(b->number());

                            if (a.valueType() == ValueType::List)
                            {
                                if (std::cmp_less(std::abs(idx), a.list().size()))
                                    push(a.list()[static_cast<std::size_t>(idx < 0 ? static_cast<long>(a.list().size()) + idx : idx)], context);
                                else
                                    throwVMError(
                                        ErrorKind::Index,
                                        fmt::format("{} out of range {} (length {})", idx, a.toString(*this), a.list().size()));
                            }
                            else if (a.valueType() == ValueType::String)
                            {
                                if (std::cmp_less(std::abs(idx), a.string().size()))
                                    push(Value(std::string(1, a.string()[static_cast<std::size_t>(idx < 0 ? static_cast<long>(a.string().size()) + idx : idx)])), context);
                                else
                                    throwVMError(
                                        ErrorKind::Index,
                                        fmt::format("{} out of range \"{}\" (length {})", idx, a.string(), a.string().size()));
                            }
                            else
                                types::generateError(
                                    "@",
                                    { { types::Contract { { types::Typedef("src", ValueType::List), types::Typedef("idx", ValueType::Number) } },
                                        types::Contract { { types::Typedef("src", ValueType::String), types::Typedef("idx", ValueType::Number) } } } },
                                    { a, *b });
                        }
                        DISPATCH();
                    }

                    TARGET(MOD)
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            types::generateError(
                                "mod",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });
                        push(Value(std::fmod(a->number(), b->number())), context);
                        DISPATCH();
                    }

                    TARGET(TYPE)
                    {
                        Value* a = popAndResolveAsPtr(context);
                        if (a == &m_undefined_value) [[unlikely]]
                            types::generateError(
                                "type",
                                { { types::Contract { { types::Typedef("value", ValueType::Any) } } } },
                                {});

                        push(Value(types_to_str[static_cast<unsigned>(a->valueType())]), context);
                        DISPATCH();
                    }

                    TARGET(HASFIELD)
                    {
                        {
                            Value *field = popAndResolveAsPtr(context), *closure = popAndResolveAsPtr(context);
                            if (closure->valueType() != ValueType::Closure || field->valueType() != ValueType::String)
                                types::generateError(
                                    "hasField",
                                    { { types::Contract { { types::Typedef("closure", ValueType::Closure), types::Typedef("field", ValueType::String) } } } },
                                    { *closure, *field });

                            auto it = std::find(m_state.m_symbols.begin(), m_state.m_symbols.end(), field->stringRef());
                            if (it == m_state.m_symbols.end())
                            {
                                push(Builtins::falseSym, context);
                                DISPATCH();
                            }

                            auto id = static_cast<std::uint16_t>(std::distance(m_state.m_symbols.begin(), it));
                            push(closure->refClosure().refScope()[id] != nullptr ? Builtins::trueSym : Builtins::falseSym, context);
                        }
                        DISPATCH();
                    }

                    TARGET(NOT)
                    {
                        Value* a = popAndResolveAsPtr(context);
                        push(!(*a) ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

#pragma endregion
                }
#if ARK_USE_COMPUTED_GOTOS
            dispatch_end:
                do
                {
                } while (false);
#endif
            }
        }
        catch (const std::exception& e)
        {
            if (fail_with_exception)
                throw;

            fmt::println("{}", e.what());
            backtrace(context);
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            // don't report a "failed" exit code so that the fuzzers can more accurately triage crashes
            m_exit_code = 0;
#else
            m_exit_code = 1;
#endif
        }
        catch (...)
        {
            if (fail_with_exception)
                throw;

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            throw;
#endif
            fmt::println("Unknown error");
            backtrace(context);
            m_exit_code = 1;
        }

        return m_exit_code;
    }

    uint16_t VM::findNearestVariableIdWithValue(const Value& value, ExecutionContext& context) const noexcept
    {
        for (auto& local : std::ranges::reverse_view(context.locals))
        {
            if (const auto id = local.idFromValue(value); id < m_state.m_symbols.size())
                return id;
        }
        return std::numeric_limits<uint16_t>::max();
    }

    void VM::throwVMError(ErrorKind kind, const std::string& message)
    {
        throw std::runtime_error(std::string(errorKinds[static_cast<std::size_t>(kind)]) + ": " + message + "\n");
    }

    void VM::backtrace(ExecutionContext& context) noexcept
    {
        const std::size_t saved_ip = context.ip;
        const std::size_t saved_pp = context.pp;
        const uint16_t saved_sp = context.sp;

        if (const uint16_t original_frame_count = context.fc; original_frame_count > 1)
        {
            // display call stack trace
            const Scope old_scope = context.locals.back();

            while (context.fc != 0)
            {
                fmt::print("[{}] ", fmt::styled(context.fc, fmt::fg(fmt::color::cyan)));
                if (context.pp != 0)
                {
                    const uint16_t id = findNearestVariableIdWithValue(
                        Value(static_cast<PageAddr_t>(context.pp)),
                        context);

                    if (id < m_state.m_symbols.size())
                        fmt::println("In function `{}'", fmt::styled(m_state.m_symbols[id], fmt::fg(fmt::color::green)));
                    else  // should never happen
                        fmt::println("In function `{}'", fmt::styled("???", fmt::fg(fmt::color::gold)));

                    Value* ip;
                    do
                    {
                        ip = popAndResolveAsPtr(context);
                    } while (ip->valueType() != ValueType::InstPtr);

                    context.ip = ip->pageAddr();
                    context.pp = pop(context)->pageAddr();
                    returnFromFuncCall(context);
                }
                else
                {
                    fmt::println("In global scope");
                    break;
                }

                if (original_frame_count - context.fc > 7)
                {
                    fmt::println("...");
                    break;
                }
            }

            // display variables values in the current scope
            fmt::println("\nCurrent scope variables values:");
            for (std::size_t i = 0, size = old_scope.size(); i < size; ++i)
            {
                fmt::println(
                    "{} = {}",
                    fmt::styled(m_state.m_symbols[old_scope.m_data[i].first], fmt::fg(fmt::color::cyan)),
                    old_scope.m_data[i].second.toString(*this));
            }

            while (context.fc != 1)
            {
                Value* tmp = pop(context);
                if (tmp->valueType() == ValueType::InstPtr)
                    --context.fc;
                *tmp = m_no_value;
            }
            // pop the PP as well
            pop(context);
        }

        std::cerr << "At IP: " << (saved_ip / 4)  // dividing by 4 because the instructions are actually on 4 bytes
                  << ", PP: " << saved_pp
                  << ", SP: " << saved_sp
                  << "\n";
    }
}
