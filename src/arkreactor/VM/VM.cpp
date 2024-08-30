#include <Ark/VM/VM.hpp>

#include <utility>
#include <numeric>
#include <limits>

#include <fmt/core.h>
#include <ranges>
#include <termcolor/proxy.hpp>
#include <Ark/Files.hpp>
#include <Ark/Utils.hpp>
#include <Ark/TypeChecker.hpp>
#include <Ark/Compiler/Instructions.hpp>

#ifdef ARK_PROFILER_MIPS
#    include <chrono>
#endif

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

        const auto it = std::ranges::remove_if(m_execution_contexts,
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

        const auto it = std::ranges::remove_if(m_futures,
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

    int VM::run() noexcept
    {
        init();
        safeRun(*m_execution_contexts[0]);
        return m_exit_code;
    }

    int VM::safeRun(ExecutionContext& context, std::size_t untilFrameCount)
    {
#ifdef ARK_PROFILER_MIPS
        auto start_time = std::chrono::system_clock::now();
        unsigned long long instructions_executed = 0;
#endif

        try
        {
            m_running = true;
            while (m_running && context.fc > untilFrameCount)
            {
                // get current instruction
                [[maybe_unused]] uint8_t padding = m_state.m_pages[context.pp][context.ip];
                uint8_t inst = m_state.m_pages[context.pp][context.ip + 1];
                auto arg = static_cast<uint16_t>((m_state.m_pages[context.pp][context.ip + 2] << 8) + m_state.m_pages[context.pp][context.ip + 3]);
                context.ip += 4;

                switch (inst)
                {
#pragma region "Instructions"

                    case LOAD_SYMBOL:
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

                        break;
                    }

                    case LOAD_CONST:
                    {
                        if (context.saved_scope && m_state.m_constants[arg].valueType() == ValueType::PageAddr)
                        {
                            push(Value(Closure(context.saved_scope.value(), m_state.m_constants[arg].pageAddr())), context);
                            context.saved_scope.reset();
                        }
                        else [[likely]]  // push internal ref
                            push(&(m_state.m_constants[arg]), context);

                        break;
                    }

                    case POP_JUMP_IF_TRUE:
                    {
                        if (*popAndResolveAsPtr(context) == Builtins::trueSym)
                            context.ip = arg * 4;  // instructions are 4 bytes
                        break;
                    }

                    case STORE:
                    {
                        Value val = *popAndResolveAsPtr(context);
                        if (Value* var = findNearestVariable(arg, context); var != nullptr) [[likely]]
                        {
                            if (var->isConst() && var->valueType() != ValueType::Reference)
                                throwVMError(ErrorKind::Mutability, fmt::format("Can not set the constant `{}' to {}", m_state.m_symbols[arg], val.toString(*this)));

                            if (var->valueType() == ValueType::Reference)
                                *var->reference() = val;
                            else [[likely]]
                            {
                                *var = val;
                                var->setConst(false);
                            }
                            break;
                        }

                        throwVMError(ErrorKind::Scope, fmt::format("Unbound variable `{}', can not change its value to {}", m_state.m_symbols[arg], val.toString(*this)));
                        break;
                    }

                    case LET:
                    {
                        // check if we are redefining a variable
                        if (auto val = (context.locals.back())[arg]; val != nullptr) [[unlikely]]
                            throwVMError(ErrorKind::Mutability, fmt::format("Can not use 'let' to redefine variable `{}'", m_state.m_symbols[arg]));

                        Value val = *popAndResolveAsPtr(context);
                        val.setConst(true);
                        context.locals.back().push_back(arg, val);

                        break;
                    }

                    case POP_JUMP_IF_FALSE:
                    {
                        if (*popAndResolveAsPtr(context) == Builtins::falseSym)
                            context.ip = arg * 4;  // instructions are 4 bytes
                        break;
                    }

                    case JUMP:
                    {
                        context.ip = arg * 4;  // instructions are 4 bytes
                        break;
                    }

                    case RET:
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

                        break;
                    }

                    case HALT:
                        m_running = false;
                        break;

                    case CALL:
                    {
                        // stack pointer + 2 because we push IP and PP
                        if (context.sp + 2u >= VMStackSize) [[unlikely]]
                            throwVMError(
                                ErrorKind::VM,
                                fmt::format(
                                    "Maximum recursion depth exceeded. You could consider rewriting your function `{}' to make use of tail-call optimization.",
                                    m_state.m_symbols[context.last_symbol]));
                        call(context, arg);
                        break;
                    }

                    case CAPTURE:
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

                        break;
                    }

                    case BUILTIN:
                    {
                        push(Builtins::builtins[arg].second, context);
                        break;
                    }

                    case MUT:
                    {
                        Value val = *popAndResolveAsPtr(context);
                        val.setConst(false);

                        // avoid adding the pair (id, _) multiple times, with different values
                        Value* local = context.locals.back()[arg];
                        if (local == nullptr) [[likely]]
                            context.locals.back().push_back(arg, val);
                        else
                            *local = val;

                        break;
                    }

                    case DEL:
                    {
                        if (Value* var = findNearestVariable(arg, context); var != nullptr)
                        {
                            if (var->valueType() == ValueType::User)
                                var->usertypeRef().del();
                            *var = Value();
                            break;
                        }

                        throwVMError(ErrorKind::Scope, fmt::format("Can not delete unbound variable `{}'", m_state.m_symbols[arg]));
                        break;
                    }

                    case SAVE_ENV:
                    {
                        context.saved_scope = context.locals.back();
                        break;
                    }

                    case GET_FIELD:
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
                            // check for CALL instruction
                            // doing a +1 on the IP to read the instruction because context.ip is already on the next instruction word (the padding)
                            if (context.ip + 1 < m_state.m_pages[context.pp].size() && m_state.m_pages[context.pp][context.ip + 1] == CALL)
                                push(Value(Closure(var->refClosure().scopePtr(), field->pageAddr())), context);
                            else
                                push(field, context);
                            break;
                        }

                        throwVMError(ErrorKind::Scope, fmt::format("`{}' isn't in the closure environment: {}", m_state.m_symbols[arg], var->refClosure().toString(*this)));
                        break;
                    }

                    case PLUGIN:
                    {
                        loadPlugin(arg, context);
                        break;
                    }

                    case LIST:
                    {
                        Value l(ValueType::List);
                        if (arg != 0)
                            l.list().reserve(arg);

                        for (uint16_t i = 0; i < arg; ++i)
                            l.push_back(*popAndResolveAsPtr(context));
                        push(std::move(l), context);
                        break;
                    }

                    case APPEND:
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
                        break;
                    }

                    case CONCAT:
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
                        break;
                    }

                    case APPEND_IN_PLACE:
                    {
                        Value* list = popAndResolveAsPtr(context);

                        if (list->isConst())
                            throwVMError(ErrorKind::Mutability, "Can not modify a constant list using `append!'");
                        if (list->valueType() != ValueType::List)
                            types::generateError(
                                "append!",
                                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                                { *list });

                        for (uint16_t i = 0; i < arg; ++i)
                            list->push_back(*popAndResolveAsPtr(context));

                        break;
                    }

                    case CONCAT_IN_PLACE:
                    {
                        Value* list = popAndResolveAsPtr(context);

                        if (list->isConst())
                            throwVMError(ErrorKind::Mutability, "Can not modify a constant list using `concat!'");
                        if (list->valueType() != ValueType::List)
                            types::generateError(
                                "concat",
                                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                                { *list });

                        for (uint16_t i = 0; i < arg; ++i)
                        {
                            Value* next = popAndResolveAsPtr(context);

                            if (next == list)
                                throwVMError(ErrorKind::Mutability, "Can not concat! a list to itself");

                            if (list->valueType() != ValueType::List || next->valueType() != ValueType::List)
                                types::generateError(
                                    "concat!",
                                    { { types::Contract { { types::Typedef("dst", ValueType::List), types::Typedef("src", ValueType::List) } } } },
                                    { *list, *next });

                            std::ranges::copy(next->list(), std::back_inserter(list->list()));
                        }

                        break;
                    }

                    case POP_LIST:
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
                        break;
                    }

                    case POP_LIST_IN_PLACE:
                    {
                        Value* list = popAndResolveAsPtr(context);
                        Value number = *popAndResolveAsPtr(context);

                        if (list->isConst())
                            throwVMError(ErrorKind::Mutability, "Can not modify a constant list using `pop!'");
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
                        break;
                    }

                    case POP:
                    {
                        pop(context);
                        break;
                    }

#pragma endregion

#pragma region "Operators"

                    case ADD:
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
                        break;
                    }

                    case SUB:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            types::generateError(
                                "-",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });
                        push(Value(a->number() - b->number()), context);
                        break;
                    }

                    case MUL:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            types::generateError(
                                "*",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });
                        push(Value(a->number() * b->number()), context);
                        break;
                    }

                    case DIV:
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
                        break;
                    }

                    case GT:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push((*a != *b && !(*a < *b)) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case LT:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push((*a < *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case LE:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push((((*a < *b) || (*a == *b)) ? Builtins::trueSym : Builtins::falseSym), context);
                        break;
                    }

                    case GE:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push(!(*a < *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case NEQ:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push((*a != *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case EQ:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push((*a == *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case LEN:
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
                        break;
                    }

                    case EMPTY:
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
                        break;
                    }

                    case TAIL:
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

                        break;
                    }

                    case HEAD:
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

                        break;
                    }

                    case ISNIL:
                    {
                        Value* a = popAndResolveAsPtr(context);
                        push((*a == Builtins::nil) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case ASSERT:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (b->valueType() != ValueType::String)
                            types::generateError(
                                "assert",
                                { { types::Contract { { types::Typedef("expr", ValueType::Any), types::Typedef("message", ValueType::String) } } } },
                                { *a, *b });

                        if (*a == Builtins::falseSym)
                            throw AssertionFailed(b->stringRef());
                        break;
                    }

                    case TO_NUM:
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
                        break;
                    }

                    case TO_STR:
                    {
                        Value* a = popAndResolveAsPtr(context);
                        push(Value(a->toString(*this)), context);
                        break;
                    }

                    case AT:
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
                        break;
                    }

                    case AND_:
                    {
                        Value *a = popAndResolveAsPtr(context), *b = popAndResolveAsPtr(context);

                        push((*a == Builtins::trueSym && *b == Builtins::trueSym) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case OR_:
                    {
                        Value *a = popAndResolveAsPtr(context), *b = popAndResolveAsPtr(context);

                        push((*b == Builtins::trueSym || *a == Builtins::trueSym) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case MOD:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            types::generateError(
                                "mod",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });

                        push(Value(std::fmod(a->number(), b->number())), context);
                        break;
                    }

                    case TYPE:
                    {
                        Value* a = popAndResolveAsPtr(context);
                        if (a == &m_undefined_value) [[unlikely]]
                            types::generateError(
                                "type",
                                { { types::Contract { { types::Typedef("value", ValueType::Any) } } } },
                                {});

                        push(Value(types_to_str[static_cast<unsigned>(a->valueType())]), context);
                        break;
                    }

                    case HASFIELD:
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
                            break;
                        }

                        auto id = static_cast<std::uint16_t>(std::distance(m_state.m_symbols.begin(), it));
                        push(closure->refClosure().refScope()[id] != nullptr ? Builtins::trueSym : Builtins::falseSym, context);

                        break;
                    }

                    case NOT:
                    {
                        Value* a = popAndResolveAsPtr(context);

                        push(!(*a) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

#pragma endregion

                    default:
                        throwVMError(ErrorKind::VM, fmt::format("Unknown instruction: {:02x}{:02x}{:04x}", padding, inst, arg));
                        break;
                }

#ifdef ARK_PROFILER_MIPS
                ++instructions_executed;
#endif
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
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
            std::cout << "Unknown error\n";
            backtrace(context);
            m_exit_code = 1;

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            throw;
#endif
        }

#ifdef ARK_PROFILER_MIPS
        auto end_time = std::chrono::system_clock::now();
        auto d = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        std::cout << "\nInstructions executed: " << instructions_executed << "\n"
                  << "Time spent: " << d.count() << " us\n"
                  << (static_cast<double>(instructions_executed) / d.count()) << " MIPS\n";
#endif

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
                std::cerr << "[" << termcolor::cyan << context.fc << termcolor::reset << "] ";
                if (context.pp != 0)
                {
                    const uint16_t id = findNearestVariableIdWithValue(
                        Value(static_cast<PageAddr_t>(context.pp)),
                        context);

                    if (id < m_state.m_symbols.size())
                        std::cerr << "In function `" << termcolor::green << m_state.m_symbols[id] << termcolor::reset << "'\n";
                    else  // should never happen
                        std::cerr << "In function `" << termcolor::yellow << "???" << termcolor::reset << "'\n";

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
                    std::cout << "In global scope\n";
                    break;
                }

                if (original_frame_count - context.fc > 7)
                {
                    std::cout << "...\n";
                    break;
                }
            }

            // display variables values in the current scope
            std::cout << "\nCurrent scope variables values:\n";
            for (std::size_t i = 0, size = old_scope.size(); i < size; ++i)
            {
                std::cerr << termcolor::cyan << m_state.m_symbols[old_scope.m_data[i].first] << termcolor::reset
                          << " = " << old_scope.m_data[i].second.toString(*this);
                std::cerr << "\n";
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

        std::cerr << termcolor::reset
                  << "At IP: " << (saved_ip / 4)  // dividing by 4 because the instructions are actually on 4 bytes
                  << ", PP: " << saved_pp
                  << ", SP: " << saved_sp
                  << "\n";
    }
}
