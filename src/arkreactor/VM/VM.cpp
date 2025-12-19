#include <Ark/VM/VM.hpp>

#include <utility>
#include <numeric>
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

#include <Ark/Utils/Files.hpp>
#include <Ark/Utils/Utils.hpp>
#include <Ark/Error/Diagnostics.hpp>
#include <Ark/TypeChecker.hpp>
#include <Ark/VM/ModuleMapping.hpp>
#include <Ark/Compiler/Instructions.hpp>

namespace Ark
{
    using namespace internal;

    namespace helper
    {
        inline Value tail(Value* a)
        {
            if (a->valueType() == ValueType::List)
            {
                if (a->constList().size() < 2)
                    return Value(ValueType::List);

                std::vector<Value> tmp(a->constList().size() - 1);
                for (std::size_t i = 1, end = a->constList().size(); i < end; ++i)
                    tmp[i - 1] = a->constList()[i];
                return Value(std::move(tmp));
            }
            if (a->valueType() == ValueType::String)
            {
                if (a->string().size() < 2)
                    return Value(ValueType::String);

                Value b { *a };
                b.stringRef().erase(b.stringRef().begin());
                return b;
            }

            throw types::TypeCheckingError(
                "tail",
                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                { *a });
        }

        inline Value head(Value* a)
        {
            if (a->valueType() == ValueType::List)
            {
                if (a->constList().empty())
                    return Builtins::nil;
                return a->constList()[0];
            }
            if (a->valueType() == ValueType::String)
            {
                if (a->string().empty())
                    return Value(ValueType::String);
                return Value(std::string(1, a->stringRef()[0]));
            }

            throw types::TypeCheckingError(
                "head",
                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                { *a });
        }

        inline Value at(Value& container, Value& index, VM& vm)
        {
            if (index.valueType() != ValueType::Number)
                throw types::TypeCheckingError(
                    "@",
                    { { types::Contract { { types::Typedef("src", ValueType::List), types::Typedef("idx", ValueType::Number) } },
                        types::Contract { { types::Typedef("src", ValueType::String), types::Typedef("idx", ValueType::Number) } } } },
                    { container, index });

            const auto num = static_cast<long>(index.number());

            if (container.valueType() == ValueType::List)
            {
                const auto i = static_cast<std::size_t>(num < 0 ? static_cast<long>(container.list().size()) + num : num);
                if (i < container.list().size())
                    return container.list()[i];
                else
                    VM::throwVMError(
                        ErrorKind::Index,
                        fmt::format("{} out of range {} (length {})", num, container.toString(vm), container.list().size()));
            }
            else if (container.valueType() == ValueType::String)
            {
                const auto i = static_cast<std::size_t>(num < 0 ? static_cast<long>(container.string().size()) + num : num);
                if (i < container.string().size())
                    return Value(std::string(1, container.string()[i]));
                else
                    VM::throwVMError(
                        ErrorKind::Index,
                        fmt::format("{} out of range \"{}\" (length {})", num, container.string(), container.string().size()));
            }
            else
                throw types::TypeCheckingError(
                    "@",
                    { { types::Contract { { types::Typedef("src", ValueType::List), types::Typedef("idx", ValueType::Number) } },
                        types::Contract { { types::Typedef("src", ValueType::String), types::Typedef("idx", ValueType::Number) } } } },
                    { container, index });
        }
    }

    VM::VM(State& state) noexcept :
        m_state(state), m_exit_code(0), m_running(false)
    {
        m_execution_contexts.emplace_back(std::make_unique<ExecutionContext>());
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
        context.locals.reserve(128);
        context.locals.emplace_back(context.scopes_storage.data(), 0);

        // loading bound stuff
        // put them in the global frame if we can, aka the first one
        for (const auto& [sym_id, value] : m_state.m_binded)
        {
            auto it = std::ranges::find(m_state.m_symbols, sym_id);
            if (it != m_state.m_symbols.end())
                context.locals[0].pushBack(static_cast<uint16_t>(std::distance(m_state.m_symbols.begin(), it)), value);
        }
    }

    Value VM::getField(Value* closure, const uint16_t id, const ExecutionContext& context)
    {
        if (closure->valueType() != ValueType::Closure)
        {
            if (context.last_symbol < m_state.m_symbols.size()) [[likely]]
                throwVMError(
                    ErrorKind::Type,
                    fmt::format(
                        "`{}' is a {}, not a Closure, can not get the field `{}' from it",
                        m_state.m_symbols[context.last_symbol],
                        std::to_string(closure->valueType()),
                        m_state.m_symbols[id]));
            else
                throwVMError(ErrorKind::Type,
                             fmt::format(
                                 "{} is not a Closure, can not get the field `{}' from it",
                                 std::to_string(closure->valueType()),
                                 m_state.m_symbols[id]));
        }

        if (Value* field = closure->refClosure().refScope()[id]; field != nullptr)
        {
            // check for CALL instruction (the instruction because context.ip is already on the next instruction word)
            if (m_state.inst(context.pp, context.ip) == CALL)
                return Value(Closure(closure->refClosure().scopePtr(), field->pageAddr()));
            else
                return *field;
        }
        else
        {
            if (!closure->refClosure().hasFieldEndingWith(m_state.m_symbols[id], *this))
                throwVMError(
                    ErrorKind::Scope,
                    fmt::format(
                        "`{0}' isn't in the closure environment: {1}",
                        m_state.m_symbols[id],
                        closure->refClosure().toString(*this)));
            throwVMError(
                ErrorKind::Scope,
                fmt::format(
                    "`{0}' isn't in the closure environment: {1}. A variable in the package might have the same name as '{0}', "
                    "and name resolution tried to fully qualify it. Rename either the variable or the capture to solve this",
                    m_state.m_symbols[id],
                    closure->refClosure().toString(*this)));
        }
    }

    Value VM::createList(const std::size_t count, internal::ExecutionContext& context)
    {
        Value l(ValueType::List);
        if (count != 0)
            l.list().reserve(count);

        for (std::size_t i = 0; i < count; ++i)
            l.push_back(*popAndResolveAsPtr(context));

        return l;
    }

    void VM::listAppendInPlace(Value* list, const std::size_t count, ExecutionContext& context)
    {
        if (list->valueType() != ValueType::List)
        {
            std::vector<Value> args = { *list };
            for (std::size_t i = 0; i < count; ++i)
                args.push_back(*popAndResolveAsPtr(context));
            throw types::TypeCheckingError(
                "append!",
                { { types::Contract { { types::Typedef("list", ValueType::List), types::Typedef("value", ValueType::Any, /* is_variadic= */ true) } } } },
                args);
        }

        for (std::size_t i = 0; i < count; ++i)
            list->push_back(*popAndResolveAsPtr(context));
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
        if (std::cmp_less(dist, MaxValue16Bits))
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
            std::vector<ScopeView::pair_t> data;
            const mapping* map = m_shared_lib_objects.back()->get<mapping* (*)()>("getFunctionsMapping")();

            std::size_t i = 0;
            while (map[i].name != nullptr)
            {
                const auto it = std::ranges::find(m_state.m_symbols, std::string(map[i].name));
                if (it != m_state.m_symbols.end())
                    data.emplace_back(static_cast<uint16_t>(std::distance(m_state.m_symbols.begin(), it)), Value(map[i].value));

                ++i;
            }

            context.locals.back().insertFront(data);
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

        ExecutionContext* ctx = nullptr;

        // Try and find a free execution context.
        // If there is only one context, this is the primary one, which can't be reused.
        // Otherwise, we can check if a context is marked as free and reserve it!
        // It is possible that all contexts are being used, thus we will create one (active by default) in that case.

        if (m_execution_contexts.size() > 1)
        {
            const auto it = std::ranges::find_if(
                m_execution_contexts,
                [](const std::unique_ptr<ExecutionContext>& context) -> bool {
                    return !context->primary && context->isFree();
                });

            if (it != m_execution_contexts.end())
            {
                ctx = it->get();
                ctx->setActive(true);
                // reset the context before using it
                ctx->sp = 0;
                ctx->saved_scope.reset();
                ctx->stacked_closure_scopes.clear();
                ctx->locals.clear();
            }
        }

        if (ctx == nullptr)
            ctx = m_execution_contexts.emplace_back(std::make_unique<ExecutionContext>()).get();

        assert(!ctx->primary && "The new context shouldn't be marked as primary!");
        assert(ctx != m_execution_contexts.front().get() && "The new context isn't really new!");

        const ExecutionContext& primary_ctx = *m_execution_contexts.front();
        ctx->locals.reserve(primary_ctx.locals.size());
        ctx->scopes_storage = primary_ctx.scopes_storage;
        ctx->stacked_closure_scopes.emplace_back(nullptr);
        ctx->fc = 1;

        for (const auto& scope_view : primary_ctx.locals)
        {
            auto& new_scope = ctx->locals.emplace_back(ctx->scopes_storage.data(), scope_view.m_start);
            for (std::size_t i = 0; i < scope_view.size(); ++i)
            {
                const auto& [id, val] = scope_view.atPos(i);
                new_scope.pushBack(id, val);
            }
        }

        return ctx;
    }

    void VM::deleteContext(ExecutionContext* ec)
    {
        const std::lock_guard lock(m_mutex);

        // 1 + 4 additional contexts, it's a bit much (~600kB per context) to have in memory
        if (m_execution_contexts.size() > 5)
        {
            const auto it =
                std::ranges::remove_if(
                    m_execution_contexts,
                    [ec](const std::unique_ptr<ExecutionContext>& ctx) {
                        return ctx.get() == ec;
                    })
                    .begin();
            m_execution_contexts.erase(it);
        }
        else
        {
            // mark the used context as ready to be used again
            for (std::size_t i = 1; i < m_execution_contexts.size(); ++i)
            {
                if (m_execution_contexts[i].get() == ec)
                {
                    ec->setActive(false);
                    break;
                }
            }
        }
    }

    Future* VM::createFuture(std::vector<Value>& args)
    {
        const std::lock_guard lock(m_mutex_futures);

        ExecutionContext* ctx = createAndGetContext();
        // so that we have access to the presumed symbol id of the function we are calling
        // assuming that the callee is always the global context
        ctx->last_symbol = m_execution_contexts.front()->last_symbol;

        m_futures.push_back(std::make_unique<Future>(ctx, this, args));
        return m_futures.back().get();
    }

    void VM::deleteFuture(Future* f)
    {
        const std::lock_guard lock(m_mutex_futures);

        std::erase_if(
            m_futures,
            [f](const std::unique_ptr<Future>& future) {
                return future.get() == f;
            });
    }

    bool VM::forceReloadPlugins() const
    {
        // load the mapping from the dynamic library
        try
        {
            for (const auto& shared_lib : m_shared_lib_objects)
            {
                const mapping* map = shared_lib->get<mapping* (*)()>("getFunctionsMapping")();
                // load the mapping data
                std::size_t i = 0;
                while (map[i].name != nullptr)
                {
                    // put it in the global frame, aka the first one
                    auto it = std::ranges::find(m_state.m_symbols, std::string(map[i].name));
                    if (it != m_state.m_symbols.end())
                        m_execution_contexts[0]->locals[0].pushBack(
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

    void VM::throwVMError(ErrorKind kind, const std::string& message)
    {
        throw std::runtime_error(std::string(errorKinds[static_cast<std::size_t>(kind)]) + ": " + message + "\n");
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

#define NEXTOPARG()                                                                                                               \
    do                                                                                                                            \
    {                                                                                                                             \
        inst = m_state.inst(context.pp, context.ip);                                                                              \
        padding = m_state.inst(context.pp, context.ip + 1);                                                                       \
        arg = static_cast<uint16_t>((m_state.inst(context.pp, context.ip + 2) << 8) +                                             \
                                    m_state.inst(context.pp, context.ip + 3));                                                    \
        context.ip += 4;                                                                                                          \
        context.inst_exec_counter = (context.inst_exec_counter + 1) % VMOverflowBufferSize;                                       \
        if (context.inst_exec_counter < 2 && context.sp >= VMStackSize)                                                           \
        {                                                                                                                         \
            if (context.pp != 0)                                                                                                  \
                throw Error("Stack overflow. You could consider rewriting your function to make use of tail-call optimization."); \
            else                                                                                                                  \
                throw Error("Stack overflow. Are you trying to call a function with too many arguments?");                        \
        }                                                                                                                         \
    } while (false)
#define DISPATCH() \
    NEXTOPARG();   \
    DISPATCH_GOTO();
#define UNPACK_ARGS()                                                                 \
    do                                                                                \
    {                                                                                 \
        secondary_arg = static_cast<uint16_t>((padding << 4) | (arg & 0xf000) >> 12); \
        primary_arg = arg & 0x0fff;                                                   \
    } while (false)

#if ARK_USE_COMPUTED_GOTOS
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
            constexpr std::array opcode_targets = {
                // cppcheck-suppress syntaxError ; cppcheck do not know about labels addresses (GCC extension)
                &&TARGET_NOP,
                &&TARGET_LOAD_SYMBOL,
                &&TARGET_LOAD_SYMBOL_BY_INDEX,
                &&TARGET_LOAD_CONST,
                &&TARGET_POP_JUMP_IF_TRUE,
                &&TARGET_STORE,
                &&TARGET_STORE_REF,
                &&TARGET_SET_VAL,
                &&TARGET_POP_JUMP_IF_FALSE,
                &&TARGET_JUMP,
                &&TARGET_RET,
                &&TARGET_HALT,
                &&TARGET_PUSH_RETURN_ADDRESS,
                &&TARGET_CALL,
                &&TARGET_CAPTURE,
                &&TARGET_RENAME_NEXT_CAPTURE,
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
                &&TARGET_SET_AT_INDEX,
                &&TARGET_SET_AT_2_INDEX,
                &&TARGET_POP,
                &&TARGET_SHORTCIRCUIT_AND,
                &&TARGET_SHORTCIRCUIT_OR,
                &&TARGET_CREATE_SCOPE,
                &&TARGET_RESET_SCOPE_JUMP,
                &&TARGET_POP_SCOPE,
                &&TARGET_GET_CURRENT_PAGE_ADDR,
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
                &&TARGET_AT_AT,
                &&TARGET_MOD,
                &&TARGET_TYPE,
                &&TARGET_HASFIELD,
                &&TARGET_NOT,
                &&TARGET_LOAD_CONST_LOAD_CONST,
                &&TARGET_LOAD_CONST_STORE,
                &&TARGET_LOAD_CONST_SET_VAL,
                &&TARGET_STORE_FROM,
                &&TARGET_STORE_FROM_INDEX,
                &&TARGET_SET_VAL_FROM,
                &&TARGET_SET_VAL_FROM_INDEX,
                &&TARGET_INCREMENT,
                &&TARGET_INCREMENT_BY_INDEX,
                &&TARGET_INCREMENT_STORE,
                &&TARGET_DECREMENT,
                &&TARGET_DECREMENT_BY_INDEX,
                &&TARGET_DECREMENT_STORE,
                &&TARGET_STORE_TAIL,
                &&TARGET_STORE_TAIL_BY_INDEX,
                &&TARGET_STORE_HEAD,
                &&TARGET_STORE_HEAD_BY_INDEX,
                &&TARGET_STORE_LIST,
                &&TARGET_SET_VAL_TAIL,
                &&TARGET_SET_VAL_TAIL_BY_INDEX,
                &&TARGET_SET_VAL_HEAD,
                &&TARGET_SET_VAL_HEAD_BY_INDEX,
                &&TARGET_CALL_BUILTIN,
                &&TARGET_CALL_BUILTIN_WITHOUT_RETURN_ADDRESS,
                &&TARGET_LT_CONST_JUMP_IF_FALSE,
                &&TARGET_LT_CONST_JUMP_IF_TRUE,
                &&TARGET_LT_SYM_JUMP_IF_FALSE,
                &&TARGET_GT_CONST_JUMP_IF_TRUE,
                &&TARGET_GT_CONST_JUMP_IF_FALSE,
                &&TARGET_GT_SYM_JUMP_IF_FALSE,
                &&TARGET_EQ_CONST_JUMP_IF_TRUE,
                &&TARGET_EQ_SYM_INDEX_JUMP_IF_TRUE,
                &&TARGET_NEQ_CONST_JUMP_IF_TRUE,
                &&TARGET_NEQ_SYM_JUMP_IF_FALSE,
                &&TARGET_CALL_SYMBOL,
                &&TARGET_CALL_CURRENT_PAGE,
                &&TARGET_GET_FIELD_FROM_SYMBOL,
                &&TARGET_GET_FIELD_FROM_SYMBOL_INDEX,
                &&TARGET_AT_SYM_SYM,
                &&TARGET_AT_SYM_INDEX_SYM_INDEX,
                &&TARGET_AT_SYM_INDEX_CONST,
                &&TARGET_CHECK_TYPE_OF,
                &&TARGET_CHECK_TYPE_OF_BY_INDEX,
                &&TARGET_APPEND_IN_PLACE_SYM,
                &&TARGET_APPEND_IN_PLACE_SYM_INDEX,
                &&TARGET_STORE_LEN,
                &&TARGET_LT_LEN_SYM_JUMP_IF_FALSE,
                &&TARGET_MUL_BY,
                &&TARGET_MUL_BY_INDEX,
                &&TARGET_MUL_SET_VAL
            };

        static_assert(opcode_targets.size() == static_cast<std::size_t>(Instruction::InstructionsCount) && "Some instructions are not implemented in the VM");
#    pragma GCC diagnostic pop
#endif

        try
        {
            uint8_t inst = 0;
            uint8_t padding = 0;
            uint16_t arg = 0;
            uint16_t primary_arg = 0;
            uint16_t secondary_arg = 0;

            m_running = true;

            DISPATCH();
            // cppcheck-suppress unreachableCode ; analysis cannot follow the chain of goto... but it works!
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
                        push(loadSymbol(arg, context), context);
                        DISPATCH();
                    }

                    TARGET(LOAD_SYMBOL_BY_INDEX)
                    {
                        push(loadSymbolFromIndex(arg, context), context);
                        DISPATCH();
                    }

                    TARGET(LOAD_CONST)
                    {
                        push(loadConstAsPtr(arg), context);
                        DISPATCH();
                    }

                    TARGET(POP_JUMP_IF_TRUE)
                    {
                        if (Value boolean = *popAndResolveAsPtr(context); !!boolean)
                            jump(arg, context);
                        DISPATCH();
                    }

                    TARGET(STORE)
                    {
                        store(arg, popAndResolveAsPtr(context), context);
                        DISPATCH();
                    }

                    TARGET(STORE_REF)
                    {
                        // Not resolving a potential ref is on purpose!
                        // This instruction is only used by functions when storing arguments
                        Value* tmp = pop(context);
                        store(arg, tmp, context);
                        DISPATCH();
                    }

                    TARGET(SET_VAL)
                    {
                        setVal(arg, popAndResolveAsPtr(context), context);
                        DISPATCH();
                    }

                    TARGET(POP_JUMP_IF_FALSE)
                    {
                        if (Value boolean = *popAndResolveAsPtr(context); !boolean)
                            jump(arg, context);
                        DISPATCH();
                    }

                    TARGET(JUMP)
                    {
                        jump(arg, context);
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
                                const Value* ip = popAndResolveAsPtr(context);
                                assert(ip->valueType() == ValueType::InstPtr && "Expected instruction pointer on the stack (is the stack trashed?)");
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

                    TARGET(PUSH_RETURN_ADDRESS)
                    {
                        push(Value(static_cast<PageAddr_t>(context.pp)), context);
                        // arg * 4 to skip over the call instruction, so that the return address points to AFTER the call
                        push(Value(ValueType::InstPtr, static_cast<PageAddr_t>(arg * 4)), context);
                        context.inst_exec_counter++;
                        DISPATCH();
                    }

                    TARGET(CALL)
                    {
                        call(context, arg);
                        if (!m_running)
                            GOTO_HALT();
                        DISPATCH();
                    }

                    TARGET(CAPTURE)
                    {
                        if (!context.saved_scope)
                            context.saved_scope = ClosureScope();

                        const Value* ptr = findNearestVariable(arg, context);
                        if (!ptr)
                            throwVMError(ErrorKind::Scope, fmt::format("Couldn't capture `{}' as it is currently unbound", m_state.m_symbols[arg]));
                        else
                        {
                            ptr = ptr->valueType() == ValueType::Reference ? ptr->reference() : ptr;
                            uint16_t id = context.capture_rename_id.value_or(arg);
                            context.saved_scope.value().push_back(id, *ptr);
                            context.capture_rename_id.reset();
                        }

                        DISPATCH();
                    }

                    TARGET(RENAME_NEXT_CAPTURE)
                    {
                        context.capture_rename_id = arg;
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
                        push(getField(var, arg, context), context);
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
                            Value l = createList(arg, context);
                            push(std::move(l), context);
                        }
                        DISPATCH();
                    }

                    TARGET(APPEND)
                    {
                        {
                            Value* list = popAndResolveAsPtr(context);
                            if (list->valueType() != ValueType::List)
                            {
                                std::vector<Value> args = { *list };
                                for (uint16_t i = 0; i < arg; ++i)
                                    args.push_back(*popAndResolveAsPtr(context));
                                throw types::TypeCheckingError(
                                    "append",
                                    { { types::Contract { { types::Typedef("list", ValueType::List), types::Typedef("value", ValueType::Any, /* variadic= */ true) } } } },
                                    args);
                            }

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
                            Value obj { *list };

                            for (uint16_t i = 0; i < arg; ++i)
                            {
                                Value* next = popAndResolveAsPtr(context);

                                if (list->valueType() != ValueType::List || next->valueType() != ValueType::List)
                                    throw types::TypeCheckingError(
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
                        listAppendInPlace(list, arg, context);
                        DISPATCH();
                    }

                    TARGET(CONCAT_IN_PLACE)
                    {
                        Value* list = popAndResolveAsPtr(context);

                        for (uint16_t i = 0; i < arg; ++i)
                        {
                            Value* next = popAndResolveAsPtr(context);

                            if (list->valueType() != ValueType::List || next->valueType() != ValueType::List)
                                throw types::TypeCheckingError(
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
                                throw types::TypeCheckingError(
                                    "pop",
                                    { { types::Contract { { types::Typedef("list", ValueType::List), types::Typedef("index", ValueType::Number) } } } },
                                    { list, number });

                            long idx = static_cast<long>(number.number());
                            idx = idx < 0 ? static_cast<long>(list.list().size()) + idx : idx;
                            if (std::cmp_greater_equal(idx, list.list().size()) || idx < 0)
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
                                throw types::TypeCheckingError(
                                    "pop!",
                                    { { types::Contract { { types::Typedef("list", ValueType::List), types::Typedef("index", ValueType::Number) } } } },
                                    { *list, number });

                            long idx = static_cast<long>(number.number());
                            idx = idx < 0 ? static_cast<long>(list->list().size()) + idx : idx;
                            if (std::cmp_greater_equal(idx, list->list().size()) || idx < 0)
                                throwVMError(
                                    ErrorKind::Index,
                                    fmt::format("pop! index ({}) out of range (list size: {})", idx, list->list().size()));

                            list->list().erase(list->list().begin() + idx);
                        }
                        DISPATCH();
                    }

                    TARGET(SET_AT_INDEX)
                    {
                        {
                            Value* list = popAndResolveAsPtr(context);
                            Value number = *popAndResolveAsPtr(context);
                            Value new_value = *popAndResolveAsPtr(context);

                            if (!list->isIndexable() || number.valueType() != ValueType::Number || (list->valueType() == ValueType::String && new_value.valueType() != ValueType::String))
                                throw types::TypeCheckingError(
                                    "@=",
                                    { { types::Contract {
                                          { types::Typedef("list", ValueType::List),
                                            types::Typedef("index", ValueType::Number),
                                            types::Typedef("new_value", ValueType::Any) } } },
                                      { types::Contract {
                                          { types::Typedef("string", ValueType::String),
                                            types::Typedef("index", ValueType::Number),
                                            types::Typedef("char", ValueType::String) } } } },
                                    { *list, number, new_value });

                            const std::size_t size = list->valueType() == ValueType::List ? list->list().size() : list->stringRef().size();
                            long idx = static_cast<long>(number.number());
                            idx = idx < 0 ? static_cast<long>(size) + idx : idx;
                            if (std::cmp_greater_equal(idx, size) || idx < 0)
                                throwVMError(
                                    ErrorKind::Index,
                                    fmt::format("@= index ({}) out of range (indexable size: {})", idx, size));

                            if (list->valueType() == ValueType::List)
                                list->list()[static_cast<std::size_t>(idx)] = new_value;
                            else
                                list->stringRef()[static_cast<std::size_t>(idx)] = new_value.string()[0];
                        }
                        DISPATCH();
                    }

                    TARGET(SET_AT_2_INDEX)
                    {
                        {
                            Value* list = popAndResolveAsPtr(context);
                            Value x = *popAndResolveAsPtr(context);
                            Value y = *popAndResolveAsPtr(context);
                            Value new_value = *popAndResolveAsPtr(context);

                            if (list->valueType() != ValueType::List || x.valueType() != ValueType::Number || y.valueType() != ValueType::Number)
                                throw types::TypeCheckingError(
                                    "@@=",
                                    { { types::Contract {
                                        { types::Typedef("list", ValueType::List),
                                          types::Typedef("x", ValueType::Number),
                                          types::Typedef("y", ValueType::Number),
                                          types::Typedef("new_value", ValueType::Any) } } } },
                                    { *list, x, y, new_value });

                            long idx_y = static_cast<long>(x.number());
                            idx_y = idx_y < 0 ? static_cast<long>(list->list().size()) + idx_y : idx_y;
                            if (std::cmp_greater_equal(idx_y, list->list().size()) || idx_y < 0)
                                throwVMError(
                                    ErrorKind::Index,
                                    fmt::format("@@= index (y: {}) out of range (list size: {})", idx_y, list->list().size()));

                            if (!list->list()[static_cast<std::size_t>(idx_y)].isIndexable() ||
                                (list->list()[static_cast<std::size_t>(idx_y)].valueType() == ValueType::String && new_value.valueType() != ValueType::String))
                                throw types::TypeCheckingError(
                                    "@@=",
                                    { { types::Contract {
                                          { types::Typedef("list", ValueType::List),
                                            types::Typedef("x", ValueType::Number),
                                            types::Typedef("y", ValueType::Number),
                                            types::Typedef("new_value", ValueType::Any) } } },
                                      { types::Contract {
                                          { types::Typedef("string", ValueType::String),
                                            types::Typedef("x", ValueType::Number),
                                            types::Typedef("y", ValueType::Number),
                                            types::Typedef("char", ValueType::String) } } } },
                                    { *list, x, y, new_value });

                            const bool is_list = list->list()[static_cast<std::size_t>(idx_y)].valueType() == ValueType::List;
                            const std::size_t size =
                                is_list
                                ? list->list()[static_cast<std::size_t>(idx_y)].list().size()
                                : list->list()[static_cast<std::size_t>(idx_y)].stringRef().size();

                            long idx_x = static_cast<long>(y.number());
                            idx_x = idx_x < 0 ? static_cast<long>(size) + idx_x : idx_x;
                            if (std::cmp_greater_equal(idx_x, size) || idx_x < 0)
                                throwVMError(
                                    ErrorKind::Index,
                                    fmt::format("@@= index (x: {}) out of range (inner indexable size: {})", idx_x, size));

                            if (is_list)
                                list->list()[static_cast<std::size_t>(idx_y)].list()[static_cast<std::size_t>(idx_x)] = new_value;
                            else
                                list->list()[static_cast<std::size_t>(idx_y)].stringRef()[static_cast<std::size_t>(idx_x)] = new_value.string()[0];
                        }
                        DISPATCH();
                    }

                    TARGET(POP)
                    {
                        pop(context);
                        DISPATCH();
                    }

                    TARGET(SHORTCIRCUIT_AND)
                    {
                        if (!*peekAndResolveAsPtr(context))
                            jump(arg, context);
                        else
                            pop(context);
                        DISPATCH();
                    }

                    TARGET(SHORTCIRCUIT_OR)
                    {
                        if (!!*peekAndResolveAsPtr(context))
                            jump(arg, context);
                        else
                            pop(context);
                        DISPATCH();
                    }

                    TARGET(CREATE_SCOPE)
                    {
                        context.locals.emplace_back(context.scopes_storage.data(), context.locals.back().storageEnd());
                        DISPATCH();
                    }

                    TARGET(RESET_SCOPE_JUMP)
                    {
                        context.locals.back().reset();
                        jump(arg, context);
                        DISPATCH();
                    }

                    TARGET(POP_SCOPE)
                    {
                        context.locals.pop_back();
                        DISPATCH();
                    }

                    TARGET(GET_CURRENT_PAGE_ADDR)
                    {
                        context.last_symbol = arg;
                        push(Value(static_cast<PageAddr_t>(context.pp)), context);
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
                            throw types::TypeCheckingError(
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
                            throw types::TypeCheckingError(
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
                            throw types::TypeCheckingError(
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
                            throw types::TypeCheckingError(
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
                        const Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push(*b < *a ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(LT)
                    {
                        const Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push(*a < *b ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(LE)
                    {
                        const Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push((((*a < *b) || (*a == *b)) ? Builtins::trueSym : Builtins::falseSym), context);
                        DISPATCH();
                    }

                    TARGET(GE)
                    {
                        const Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push(!(*a < *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(NEQ)
                    {
                        const Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push(*a != *b ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(EQ)
                    {
                        const Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        push(*a == *b ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(LEN)
                    {
                        const Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() == ValueType::List)
                            push(Value(static_cast<int>(a->constList().size())), context);
                        else if (a->valueType() == ValueType::String)
                            push(Value(static_cast<int>(a->string().size())), context);
                        else
                            throw types::TypeCheckingError(
                                "len",
                                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                { *a });
                        DISPATCH();
                    }

                    TARGET(EMPTY)
                    {
                        const Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() == ValueType::List)
                            push(a->constList().empty() ? Builtins::trueSym : Builtins::falseSym, context);
                        else if (a->valueType() == ValueType::String)
                            push(a->string().empty() ? Builtins::trueSym : Builtins::falseSym, context);
                        else
                            throw types::TypeCheckingError(
                                "empty?",
                                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                { *a });
                        DISPATCH();
                    }

                    TARGET(TAIL)
                    {
                        Value* const a = popAndResolveAsPtr(context);
                        push(helper::tail(a), context);
                        DISPATCH();
                    }

                    TARGET(HEAD)
                    {
                        Value* const a = popAndResolveAsPtr(context);
                        push(helper::head(a), context);
                        DISPATCH();
                    }

                    TARGET(ISNIL)
                    {
                        const Value* a = popAndResolveAsPtr(context);
                        push((*a == Builtins::nil) ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

                    TARGET(ASSERT)
                    {
                        Value* const b = popAndResolveAsPtr(context);
                        Value* const a = popAndResolveAsPtr(context);

                        if (b->valueType() != ValueType::String)
                            throw types::TypeCheckingError(
                                "assert",
                                { { types::Contract { { types::Typedef("expr", ValueType::Any), types::Typedef("message", ValueType::String) } } } },
                                { *a, *b });

                        if (*a == Builtins::falseSym)
                            throw AssertionFailed(b->stringRef());
                        DISPATCH();
                    }

                    TARGET(TO_NUM)
                    {
                        const Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::String)
                            throw types::TypeCheckingError(
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
                        const Value* a = popAndResolveAsPtr(context);
                        push(Value(a->toString(*this)), context);
                        DISPATCH();
                    }

                    TARGET(AT)
                    {
                        Value& b = *popAndResolveAsPtr(context);
                        Value& a = *popAndResolveAsPtr(context);
                        push(helper::at(a, b, *this), context);
                        DISPATCH();
                    }

                    TARGET(AT_AT)
                    {
                        {
                            const Value* x = popAndResolveAsPtr(context);
                            const Value* y = popAndResolveAsPtr(context);
                            Value& list = *popAndResolveAsPtr(context);

                            if (y->valueType() != ValueType::Number || x->valueType() != ValueType::Number ||
                                list.valueType() != ValueType::List)
                                throw types::TypeCheckingError(
                                    "@@",
                                    { { types::Contract {
                                        { types::Typedef("src", ValueType::List),
                                          types::Typedef("y", ValueType::Number),
                                          types::Typedef("x", ValueType::Number) } } } },
                                    { list, *y, *x });

                            long idx_y = static_cast<long>(y->number());
                            idx_y = idx_y < 0 ? static_cast<long>(list.list().size()) + idx_y : idx_y;
                            if (std::cmp_greater_equal(idx_y, list.list().size()) || idx_y < 0)
                                throwVMError(
                                    ErrorKind::Index,
                                    fmt::format("@@ index ({}) out of range (list size: {})", idx_y, list.list().size()));

                            const bool is_list = list.list()[static_cast<std::size_t>(idx_y)].valueType() == ValueType::List;
                            const std::size_t size =
                                is_list
                                ? list.list()[static_cast<std::size_t>(idx_y)].list().size()
                                : list.list()[static_cast<std::size_t>(idx_y)].stringRef().size();

                            long idx_x = static_cast<long>(x->number());
                            idx_x = idx_x < 0 ? static_cast<long>(size) + idx_x : idx_x;
                            if (std::cmp_greater_equal(idx_x, size) || idx_x < 0)
                                throwVMError(
                                    ErrorKind::Index,
                                    fmt::format("@@ index (x: {}) out of range (inner indexable size: {})", idx_x, size));

                            if (is_list)
                                push(list.list()[static_cast<std::size_t>(idx_y)].list()[static_cast<std::size_t>(idx_x)], context);
                            else
                                push(Value(std::string(1, list.list()[static_cast<std::size_t>(idx_y)].stringRef()[static_cast<std::size_t>(idx_x)])), context);
                        }
                        DISPATCH();
                    }

                    TARGET(MOD)
                    {
                        const Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);
                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            throw types::TypeCheckingError(
                                "mod",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });
                        push(Value(std::fmod(a->number(), b->number())), context);
                        DISPATCH();
                    }

                    TARGET(TYPE)
                    {
                        const Value* a = popAndResolveAsPtr(context);
                        push(Value(std::to_string(a->valueType())), context);
                        DISPATCH();
                    }

                    TARGET(HASFIELD)
                    {
                        {
                            Value* const field = popAndResolveAsPtr(context);
                            Value* const closure = popAndResolveAsPtr(context);
                            if (closure->valueType() != ValueType::Closure || field->valueType() != ValueType::String)
                                throw types::TypeCheckingError(
                                    "hasField",
                                    { { types::Contract { { types::Typedef("closure", ValueType::Closure), types::Typedef("field", ValueType::String) } } } },
                                    { *closure, *field });

                            auto it = std::ranges::find(m_state.m_symbols, field->stringRef());
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
                        const Value* a = popAndResolveAsPtr(context);
                        push(!(*a) ? Builtins::trueSym : Builtins::falseSym, context);
                        DISPATCH();
                    }

#pragma endregion

#pragma region "Super Instructions"
                    TARGET(LOAD_CONST_LOAD_CONST)
                    {
                        UNPACK_ARGS();
                        push(loadConstAsPtr(primary_arg), context);
                        push(loadConstAsPtr(secondary_arg), context);
                        context.inst_exec_counter++;
                        DISPATCH();
                    }

                    TARGET(LOAD_CONST_STORE)
                    {
                        UNPACK_ARGS();
                        store(secondary_arg, loadConstAsPtr(primary_arg), context);
                        DISPATCH();
                    }

                    TARGET(LOAD_CONST_SET_VAL)
                    {
                        UNPACK_ARGS();
                        setVal(secondary_arg, loadConstAsPtr(primary_arg), context);
                        DISPATCH();
                    }

                    TARGET(STORE_FROM)
                    {
                        UNPACK_ARGS();
                        store(secondary_arg, loadSymbol(primary_arg, context), context);
                        DISPATCH();
                    }

                    TARGET(STORE_FROM_INDEX)
                    {
                        UNPACK_ARGS();
                        store(secondary_arg, loadSymbolFromIndex(primary_arg, context), context);
                        DISPATCH();
                    }

                    TARGET(SET_VAL_FROM)
                    {
                        UNPACK_ARGS();
                        setVal(secondary_arg, loadSymbol(primary_arg, context), context);
                        DISPATCH();
                    }

                    TARGET(SET_VAL_FROM_INDEX)
                    {
                        UNPACK_ARGS();
                        setVal(secondary_arg, loadSymbolFromIndex(primary_arg, context), context);
                        DISPATCH();
                    }

                    TARGET(INCREMENT)
                    {
                        UNPACK_ARGS();
                        {
                            Value* var = loadSymbol(primary_arg, context);

                            // use internal reference, shouldn't break anything so far, unless it's already a ref
                            if (var->valueType() == ValueType::Reference)
                                var = var->reference();

                            if (var->valueType() == ValueType::Number)
                                push(Value(var->number() + secondary_arg), context);
                            else
                                throw types::TypeCheckingError(
                                    "+",
                                    { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                    { *var, Value(secondary_arg) });
                        }
                        DISPATCH();
                    }

                    TARGET(INCREMENT_BY_INDEX)
                    {
                        UNPACK_ARGS();
                        {
                            Value* var = loadSymbolFromIndex(primary_arg, context);

                            // use internal reference, shouldn't break anything so far, unless it's already a ref
                            if (var->valueType() == ValueType::Reference)
                                var = var->reference();

                            if (var->valueType() == ValueType::Number)
                                push(Value(var->number() + secondary_arg), context);
                            else
                                throw types::TypeCheckingError(
                                    "+",
                                    { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                    { *var, Value(secondary_arg) });
                        }
                        DISPATCH();
                    }

                    TARGET(INCREMENT_STORE)
                    {
                        UNPACK_ARGS();
                        {
                            Value* var = loadSymbol(primary_arg, context);

                            // use internal reference, shouldn't break anything so far, unless it's already a ref
                            if (var->valueType() == ValueType::Reference)
                                var = var->reference();

                            if (var->valueType() == ValueType::Number)
                            {
                                auto val = Value(var->number() + secondary_arg);
                                setVal(primary_arg, &val, context);
                            }
                            else
                                throw types::TypeCheckingError(
                                    "+",
                                    { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                    { *var, Value(secondary_arg) });
                        }
                        DISPATCH();
                    }

                    TARGET(DECREMENT)
                    {
                        UNPACK_ARGS();
                        {
                            Value* var = loadSymbol(primary_arg, context);

                            // use internal reference, shouldn't break anything so far, unless it's already a ref
                            if (var->valueType() == ValueType::Reference)
                                var = var->reference();

                            if (var->valueType() == ValueType::Number)
                                push(Value(var->number() - secondary_arg), context);
                            else
                                throw types::TypeCheckingError(
                                    "-",
                                    { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                    { *var, Value(secondary_arg) });
                        }
                        DISPATCH();
                    }

                    TARGET(DECREMENT_BY_INDEX)
                    {
                        UNPACK_ARGS();
                        {
                            Value* var = loadSymbolFromIndex(primary_arg, context);

                            // use internal reference, shouldn't break anything so far, unless it's already a ref
                            if (var->valueType() == ValueType::Reference)
                                var = var->reference();

                            if (var->valueType() == ValueType::Number)
                                push(Value(var->number() - secondary_arg), context);
                            else
                                throw types::TypeCheckingError(
                                    "-",
                                    { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                    { *var, Value(secondary_arg) });
                        }
                        DISPATCH();
                    }

                    TARGET(DECREMENT_STORE)
                    {
                        UNPACK_ARGS();
                        {
                            Value* var = loadSymbol(primary_arg, context);

                            // use internal reference, shouldn't break anything so far, unless it's already a ref
                            if (var->valueType() == ValueType::Reference)
                                var = var->reference();

                            if (var->valueType() == ValueType::Number)
                            {
                                auto val = Value(var->number() - secondary_arg);
                                setVal(primary_arg, &val, context);
                            }
                            else
                                throw types::TypeCheckingError(
                                    "-",
                                    { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                    { *var, Value(secondary_arg) });
                        }
                        DISPATCH();
                    }

                    TARGET(STORE_TAIL)
                    {
                        UNPACK_ARGS();
                        {
                            Value* list = loadSymbol(primary_arg, context);
                            Value tail = helper::tail(list);
                            store(secondary_arg, &tail, context);
                        }
                        DISPATCH();
                    }

                    TARGET(STORE_TAIL_BY_INDEX)
                    {
                        UNPACK_ARGS();
                        {
                            Value* list = loadSymbolFromIndex(primary_arg, context);
                            Value tail = helper::tail(list);
                            store(secondary_arg, &tail, context);
                        }
                        DISPATCH();
                    }

                    TARGET(STORE_HEAD)
                    {
                        UNPACK_ARGS();
                        {
                            Value* list = loadSymbol(primary_arg, context);
                            Value head = helper::head(list);
                            store(secondary_arg, &head, context);
                        }
                        DISPATCH();
                    }

                    TARGET(STORE_HEAD_BY_INDEX)
                    {
                        UNPACK_ARGS();
                        {
                            Value* list = loadSymbolFromIndex(primary_arg, context);
                            Value head = helper::head(list);
                            store(secondary_arg, &head, context);
                        }
                        DISPATCH();
                    }

                    TARGET(STORE_LIST)
                    {
                        UNPACK_ARGS();
                        {
                            Value l = createList(primary_arg, context);
                            store(secondary_arg, &l, context);
                        }
                        DISPATCH();
                    }

                    TARGET(SET_VAL_TAIL)
                    {
                        UNPACK_ARGS();
                        {
                            Value* list = loadSymbol(primary_arg, context);
                            Value tail = helper::tail(list);
                            setVal(secondary_arg, &tail, context);
                        }
                        DISPATCH();
                    }

                    TARGET(SET_VAL_TAIL_BY_INDEX)
                    {
                        UNPACK_ARGS();
                        {
                            Value* list = loadSymbolFromIndex(primary_arg, context);
                            Value tail = helper::tail(list);
                            setVal(secondary_arg, &tail, context);
                        }
                        DISPATCH();
                    }

                    TARGET(SET_VAL_HEAD)
                    {
                        UNPACK_ARGS();
                        {
                            Value* list = loadSymbol(primary_arg, context);
                            Value head = helper::head(list);
                            setVal(secondary_arg, &head, context);
                        }
                        DISPATCH();
                    }

                    TARGET(SET_VAL_HEAD_BY_INDEX)
                    {
                        UNPACK_ARGS();
                        {
                            Value* list = loadSymbolFromIndex(primary_arg, context);
                            Value head = helper::head(list);
                            setVal(secondary_arg, &head, context);
                        }
                        DISPATCH();
                    }

                    TARGET(CALL_BUILTIN)
                    {
                        UNPACK_ARGS();
                        // no stack size check because we do not push IP/PP since we are just calling a builtin
                        callBuiltin(context, Builtins::builtins[primary_arg].second, secondary_arg);
                        if (!m_running)
                            GOTO_HALT();
                        DISPATCH();
                    }

                    TARGET(CALL_BUILTIN_WITHOUT_RETURN_ADDRESS)
                    {
                        UNPACK_ARGS();
                        // no stack size check because we do not push IP/PP since we are just calling a builtin
                        callBuiltin(context, Builtins::builtins[primary_arg].second, secondary_arg, /* remove_return_address= */ false);
                        if (!m_running)
                            GOTO_HALT();
                        DISPATCH();
                    }

                    TARGET(LT_CONST_JUMP_IF_FALSE)
                    {
                        UNPACK_ARGS();
                        const Value* sym = popAndResolveAsPtr(context);
                        if (!(*sym < *loadConstAsPtr(primary_arg)))
                            jump(secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(LT_CONST_JUMP_IF_TRUE)
                    {
                        UNPACK_ARGS();
                        const Value* sym = popAndResolveAsPtr(context);
                        if (*sym < *loadConstAsPtr(primary_arg))
                            jump(secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(LT_SYM_JUMP_IF_FALSE)
                    {
                        UNPACK_ARGS();
                        const Value* sym = popAndResolveAsPtr(context);
                        if (!(*sym < *loadSymbol(primary_arg, context)))
                            jump(secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(GT_CONST_JUMP_IF_TRUE)
                    {
                        UNPACK_ARGS();
                        const Value* sym = popAndResolveAsPtr(context);
                        const Value* cst = loadConstAsPtr(primary_arg);
                        if (*cst < *sym)
                            jump(secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(GT_CONST_JUMP_IF_FALSE)
                    {
                        UNPACK_ARGS();
                        const Value* sym = popAndResolveAsPtr(context);
                        const Value* cst = loadConstAsPtr(primary_arg);
                        if (!(*cst < *sym))
                            jump(secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(GT_SYM_JUMP_IF_FALSE)
                    {
                        UNPACK_ARGS();
                        const Value* sym = popAndResolveAsPtr(context);
                        const Value* rhs = loadSymbol(primary_arg, context);
                        if (!(*rhs < *sym))
                            jump(secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(EQ_CONST_JUMP_IF_TRUE)
                    {
                        UNPACK_ARGS();
                        const Value* sym = popAndResolveAsPtr(context);
                        if (*sym == *loadConstAsPtr(primary_arg))
                            jump(secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(EQ_SYM_INDEX_JUMP_IF_TRUE)
                    {
                        UNPACK_ARGS();
                        const Value* sym = popAndResolveAsPtr(context);
                        if (*sym == *loadSymbolFromIndex(primary_arg, context))
                            jump(secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(NEQ_CONST_JUMP_IF_TRUE)
                    {
                        UNPACK_ARGS();
                        const Value* sym = popAndResolveAsPtr(context);
                        if (*sym != *loadConstAsPtr(primary_arg))
                            jump(secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(NEQ_SYM_JUMP_IF_FALSE)
                    {
                        UNPACK_ARGS();
                        const Value* sym = popAndResolveAsPtr(context);
                        if (*sym == *loadSymbol(primary_arg, context))
                            jump(secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(CALL_SYMBOL)
                    {
                        UNPACK_ARGS();
                        call(context, secondary_arg, loadSymbol(primary_arg, context));
                        if (!m_running)
                            GOTO_HALT();
                        DISPATCH();
                    }

                    TARGET(CALL_CURRENT_PAGE)
                    {
                        UNPACK_ARGS();
                        context.last_symbol = primary_arg;
                        call(context, secondary_arg, /* function_ptr= */ nullptr, /* or_address= */ static_cast<PageAddr_t>(context.pp));
                        if (!m_running)
                            GOTO_HALT();
                        DISPATCH();
                    }

                    TARGET(GET_FIELD_FROM_SYMBOL)
                    {
                        UNPACK_ARGS();
                        push(getField(loadSymbol(primary_arg, context), secondary_arg, context), context);
                        DISPATCH();
                    }

                    TARGET(GET_FIELD_FROM_SYMBOL_INDEX)
                    {
                        UNPACK_ARGS();
                        push(getField(loadSymbolFromIndex(primary_arg, context), secondary_arg, context), context);
                        DISPATCH();
                    }

                    TARGET(AT_SYM_SYM)
                    {
                        UNPACK_ARGS();
                        push(helper::at(*loadSymbol(primary_arg, context), *loadSymbol(secondary_arg, context), *this), context);
                        DISPATCH();
                    }

                    TARGET(AT_SYM_INDEX_SYM_INDEX)
                    {
                        UNPACK_ARGS();
                        push(helper::at(*loadSymbolFromIndex(primary_arg, context), *loadSymbolFromIndex(secondary_arg, context), *this), context);
                        DISPATCH();
                    }

                    TARGET(AT_SYM_INDEX_CONST)
                    {
                        UNPACK_ARGS();
                        push(helper::at(*loadSymbolFromIndex(primary_arg, context), *loadConstAsPtr(secondary_arg), *this), context);
                        DISPATCH();
                    }

                    TARGET(CHECK_TYPE_OF)
                    {
                        UNPACK_ARGS();
                        const Value* sym = loadSymbol(primary_arg, context);
                        const Value* cst = loadConstAsPtr(secondary_arg);
                        push(
                            cst->valueType() == ValueType::String &&
                                    std::to_string(sym->valueType()) == cst->string()
                                ? Builtins::trueSym
                                : Builtins::falseSym,
                            context);
                        DISPATCH();
                    }

                    TARGET(CHECK_TYPE_OF_BY_INDEX)
                    {
                        UNPACK_ARGS();
                        const Value* sym = loadSymbolFromIndex(primary_arg, context);
                        const Value* cst = loadConstAsPtr(secondary_arg);
                        push(
                            cst->valueType() == ValueType::String &&
                                    std::to_string(sym->valueType()) == cst->string()
                                ? Builtins::trueSym
                                : Builtins::falseSym,
                            context);
                        DISPATCH();
                    }

                    TARGET(APPEND_IN_PLACE_SYM)
                    {
                        UNPACK_ARGS();
                        listAppendInPlace(loadSymbol(primary_arg, context), secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(APPEND_IN_PLACE_SYM_INDEX)
                    {
                        UNPACK_ARGS();
                        listAppendInPlace(loadSymbolFromIndex(primary_arg, context), secondary_arg, context);
                        DISPATCH();
                    }

                    TARGET(STORE_LEN)
                    {
                        UNPACK_ARGS();
                        {
                            Value* a = loadSymbolFromIndex(primary_arg, context);
                            Value len;
                            if (a->valueType() == ValueType::List)
                                len = Value(static_cast<int>(a->constList().size()));
                            else if (a->valueType() == ValueType::String)
                                len = Value(static_cast<int>(a->string().size()));
                            else
                                throw types::TypeCheckingError(
                                    "len",
                                    { { types::Contract { { types::Typedef("value", ValueType::List) } },
                                        types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                    { *a });
                            store(secondary_arg, &len, context);
                        }
                        DISPATCH();
                    }

                    TARGET(LT_LEN_SYM_JUMP_IF_FALSE)
                    {
                        UNPACK_ARGS();
                        {
                            const Value* sym = loadSymbol(primary_arg, context);
                            Value size;

                            if (sym->valueType() == ValueType::List)
                                size = Value(static_cast<int>(sym->constList().size()));
                            else if (sym->valueType() == ValueType::String)
                                size = Value(static_cast<int>(sym->string().size()));
                            else
                                throw types::TypeCheckingError(
                                    "len",
                                    { { types::Contract { { types::Typedef("value", ValueType::List) } },
                                        types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                    { *sym });

                            if (!(*popAndResolveAsPtr(context) < size))
                                jump(secondary_arg, context);
                        }
                        DISPATCH();
                    }

                    TARGET(MUL_BY)
                    {
                        UNPACK_ARGS();
                        {
                            Value* var = loadSymbol(primary_arg, context);
                            const int other = static_cast<int>(secondary_arg) - 2048;

                            // use internal reference, shouldn't break anything so far, unless it's already a ref
                            if (var->valueType() == ValueType::Reference)
                                var = var->reference();

                            if (var->valueType() == ValueType::Number)
                                push(Value(var->number() * other), context);
                            else
                                throw types::TypeCheckingError(
                                    "*",
                                    { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                    { *var, Value(other) });
                        }
                        DISPATCH();
                    }

                    TARGET(MUL_BY_INDEX)
                    {
                        UNPACK_ARGS();
                        {
                            Value* var = loadSymbolFromIndex(primary_arg, context);
                            const int other = static_cast<int>(secondary_arg) - 2048;

                            // use internal reference, shouldn't break anything so far, unless it's already a ref
                            if (var->valueType() == ValueType::Reference)
                                var = var->reference();

                            if (var->valueType() == ValueType::Number)
                                push(Value(var->number() * other), context);
                            else
                                throw types::TypeCheckingError(
                                    "*",
                                    { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                    { *var, Value(other) });
                        }
                        DISPATCH();
                    }

                    TARGET(MUL_SET_VAL)
                    {
                        UNPACK_ARGS();
                        {
                            Value* var = loadSymbol(primary_arg, context);
                            const int other = static_cast<int>(secondary_arg) - 2048;

                            // use internal reference, shouldn't break anything so far, unless it's already a ref
                            if (var->valueType() == ValueType::Reference)
                                var = var->reference();

                            if (var->valueType() == ValueType::Number)
                            {
                                auto val = Value(var->number() * other);
                                setVal(primary_arg, &val, context);
                            }
                            else
                                throw types::TypeCheckingError(
                                    "*",
                                    { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                    { *var, Value(other) });
                        }
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
        catch (const Error& e)
        {
            if (fail_with_exception)
            {
                std::stringstream stream;
                backtrace(context, stream, /* colorize= */ false);
                // It's important we have an Ark::Error here, as the constructor for NestedError
                // does more than just aggregate error messages, hence the code duplication.
                throw NestedError(e, stream.str(), *this);
            }
            else
                showBacktraceWithException(Error(e.details(/* colorize= */ true, *this)), context);
        }
        catch (const std::exception& e)
        {
            if (fail_with_exception)
            {
                std::stringstream stream;
                backtrace(context, stream, /* colorize= */ false);
                throw NestedError(e, stream.str());
            }
            else
                showBacktraceWithException(e, context);
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
        return MaxValue16Bits;
    }

    void VM::throwArityError(std::size_t passed_arg_count, std::size_t expected_arg_count, internal::ExecutionContext& context)
    {
        std::vector<std::string> arg_names;
        arg_names.reserve(expected_arg_count + 1);
        if (expected_arg_count > 0)
            arg_names.emplace_back("");  // for formatting, so that we have a space between the function and the args

        std::size_t index = 0;
        while (m_state.inst(context.pp, index) == STORE ||
               m_state.inst(context.pp, index) == STORE_REF)
        {
            const auto id = static_cast<uint16_t>((m_state.inst(context.pp, index + 2) << 8) + m_state.inst(context.pp, index + 3));
            arg_names.push_back(m_state.m_symbols[id]);
            index += 4;
        }
        // we only the blank space for formatting and no arg names, probably because of a CALL_BUILTIN_WITHOUT_RETURN_ADDRESS
        if (arg_names.size() == 1 && index == 0)
        {
            assert(m_state.inst(context.pp, 0) == CALL_BUILTIN_WITHOUT_RETURN_ADDRESS && "expected a CALL_BUILTIN_WITHOUT_RETURN_ADDRESS instruction or STORE instructions");
            for (std::size_t i = 0; i < expected_arg_count; ++i)
                arg_names.push_back(std::string(1, static_cast<char>('a' + i)));
        }

        std::vector<std::string> arg_vals;
        arg_vals.reserve(passed_arg_count + 1);
        if (passed_arg_count > 0)
            arg_vals.emplace_back("");  // for formatting, so that we have a space between the function and the args

        for (std::size_t i = 0; i < passed_arg_count && i + 1 <= context.sp; ++i)
            // -1 on the stack because we always point to the next available slot
            arg_vals.push_back(context.stack[context.sp - i - 1].toString(*this));

        // set ip/pp to the callee location so that the error can pinpoint the line
        // where the bad call happened
        if (context.sp >= 2 + passed_arg_count)
        {
            context.ip = context.stack[context.sp - 1 - passed_arg_count].pageAddr();
            context.pp = context.stack[context.sp - 2 - passed_arg_count].pageAddr();
            returnFromFuncCall(context);
        }

        std::string function_name = (context.last_symbol < m_state.m_symbols.size())
            ? m_state.m_symbols[context.last_symbol]
            : Value(static_cast<PageAddr_t>(context.pp)).toString(*this);

        throwVMError(
            ErrorKind::Arity,
            fmt::format(
                "When calling `({}{})', received {} argument{}, but expected {}: `({}{})'",
                function_name,
                fmt::join(arg_vals, " "),
                passed_arg_count,
                passed_arg_count > 1 ? "s" : "",
                expected_arg_count,
                function_name,
                fmt::join(arg_names, " ")));
    }

    void VM::showBacktraceWithException(const std::exception& e, internal::ExecutionContext& context)
    {
        std::string text = e.what();
        if (!text.empty() && text.back() != '\n')
            text += '\n';
        fmt::println("{}", text);
        backtrace(context);
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        // don't report a "failed" exit code so that the fuzzers can more accurately triage crashes
        m_exit_code = 0;
#else
        m_exit_code = 1;
#endif
    }

    std::optional<InstLoc> VM::findSourceLocation(const std::size_t ip, const std::size_t pp) const
    {
        std::optional<InstLoc> match = std::nullopt;

        for (const auto location : m_state.m_inst_locations)
        {
            if (location.page_pointer == pp && !match)
                match = location;

            // select the best match: we want to find the location that's nearest our instruction pointer,
            // but not equal to it as the IP will always be pointing to the next instruction,
            // not yet executed. Thus, the erroneous instruction is the previous one.
            if (location.page_pointer == pp && match && location.inst_pointer < ip / 4)
                match = location;

            // early exit because we won't find anything better, as inst locations are ordered by ascending (pp, ip)
            if (location.page_pointer > pp || (location.page_pointer == pp && location.inst_pointer >= ip / 4))
                break;
        }

        return match;
    }

    std::string VM::debugShowSource() const
    {
        const auto& context = m_execution_contexts.front();
        auto maybe_source_loc = findSourceLocation(context->ip, context->pp);
        if (maybe_source_loc)
        {
            const auto filename = m_state.m_filenames[maybe_source_loc->filename_id];
            return fmt::format("{}:{} -- IP: {}, PP: {}", filename, maybe_source_loc->line + 1, maybe_source_loc->inst_pointer, maybe_source_loc->page_pointer);
        }
        return "No source location found";
    }

    void VM::backtrace(ExecutionContext& context, std::ostream& os, const bool colorize)
    {
        const std::size_t saved_ip = context.ip;
        const std::size_t saved_pp = context.pp;
        const uint16_t saved_sp = context.sp;
        constexpr std::size_t max_consecutive_traces = 7;

        const auto maybe_location = findSourceLocation(context.ip, context.pp);
        if (maybe_location)
        {
            const auto filename = m_state.m_filenames[maybe_location->filename_id];

            if (Utils::fileExists(filename))
                Diagnostics::makeContext(
                    Diagnostics::ErrorLocation {
                        .filename = filename,
                        .start = FilePos { .line = maybe_location->line, .column = 0 },
                        .end = std::nullopt },
                    os,
                    /* maybe_context= */ std::nullopt,
                    /* colorize= */ colorize);
            fmt::println(os, "");
        }

        if (context.fc > 1)
        {
            // display call stack trace
            const ScopeView old_scope = context.locals.back();

            std::string previous_trace;
            std::size_t displayed_traces = 0;
            std::size_t consecutive_similar_traces = 0;

            while (context.fc != 0 && context.pp != 0)
            {
                const auto maybe_call_loc = findSourceLocation(context.ip, context.pp);
                const auto loc_as_text = maybe_call_loc ? fmt::format(" ({}:{})", m_state.m_filenames[maybe_call_loc->filename_id], maybe_call_loc->line + 1) : "";

                const uint16_t id = findNearestVariableIdWithValue(
                    Value(static_cast<PageAddr_t>(context.pp)),
                    context);
                const std::string& func_name = (id < m_state.m_symbols.size()) ? m_state.m_symbols[id] : "???";

                if (func_name + loc_as_text != previous_trace)
                {
                    fmt::println(
                        os,
                        "[{:4}] In function `{}'{}",
                        fmt::styled(context.fc, colorize ? fmt::fg(fmt::color::cyan) : fmt::text_style()),
                        fmt::styled(func_name, colorize ? fmt::fg(fmt::color::green) : fmt::text_style()),
                        loc_as_text);
                    previous_trace = func_name + loc_as_text;
                    ++displayed_traces;
                    consecutive_similar_traces = 0;
                }
                else if (consecutive_similar_traces == 0)
                {
                    fmt::println(os, "       ...");
                    ++consecutive_similar_traces;
                }

                const Value* ip;
                do
                {
                    ip = popAndResolveAsPtr(context);
                } while (ip->valueType() != ValueType::InstPtr);

                context.ip = ip->pageAddr();
                context.pp = pop(context)->pageAddr();
                returnFromFuncCall(context);

                if (displayed_traces > max_consecutive_traces)
                {
                    fmt::println(os, "       ...");
                    break;
                }
            }

            if (context.pp == 0)
            {
                const auto maybe_call_loc = findSourceLocation(context.ip, context.pp);
                const auto loc_as_text = maybe_call_loc ? fmt::format(" ({}:{})", m_state.m_filenames[maybe_call_loc->filename_id], maybe_call_loc->line + 1) : "";
                fmt::println(os, "[{:4}] In global scope{}", fmt::styled(context.fc, colorize ? fmt::fg(fmt::color::cyan) : fmt::text_style()), loc_as_text);
            }

            // display variables values in the current scope
            fmt::println(os, "\nCurrent scope variables values:");
            for (std::size_t i = 0, size = old_scope.size(); i < size; ++i)
            {
                fmt::println(
                    os,
                    "{} = {}",
                    fmt::styled(m_state.m_symbols[old_scope.atPos(i).first], colorize ? fmt::fg(fmt::color::cyan) : fmt::text_style()),
                    old_scope.atPos(i).second.toString(*this));
            }
        }

        fmt::println(
            os,
            "At IP: {}, PP: {}, SP: {}",
            // dividing by 4 because the instructions are actually on 4 bytes
            fmt::styled(saved_ip / 4, colorize ? fmt::fg(fmt::color::cyan) : fmt::text_style()),
            fmt::styled(saved_pp, colorize ? fmt::fg(fmt::color::green) : fmt::text_style()),
            fmt::styled(saved_sp, colorize ? fmt::fg(fmt::color::yellow) : fmt::text_style()));
    }
}
