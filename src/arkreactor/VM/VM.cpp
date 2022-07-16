#define NOMINMAX

#include <Ark/VM/VM.hpp>

#include <numeric>
#include <limits>

#include <termcolor/termcolor.hpp>
#include <Ark/Files.hpp>
#include <Ark/Utils.hpp>
#include <Ark/TypeChecker.hpp>

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
        m_state(state), m_exit_code(0),
        m_running(false),
        m_user_pointer(nullptr)
    {
        m_execution_contexts.emplace_back(std::make_unique<ExecutionContext>())->locals.reserve(4);
    }

    void VM::init() noexcept
    {
        ExecutionContext& context = *m_execution_contexts.back();

        context.sp = 0;
        context.fc = 1;

        m_shared_lib_objects.clear();
        context.scope_count_to_delete.clear();
        context.scope_count_to_delete.emplace_back(0);

        context.saved_scope.reset();
        m_exit_code = 0;

        context.locals.clear();
        createNewScope(context);

        if (context.locals.size() == 0)
        {
            // if persistance is set but not scopes are present, add one
            createNewScope(context);
        }

        // loading binded stuff
        // put them in the global frame if we can, aka the first one
        for (auto name_val : m_state.m_binded)
        {
            auto it = std::find(m_state.m_symbols.begin(), m_state.m_symbols.end(), name_val.first);
            if (it != m_state.m_symbols.end())
                (*context.locals[0]).push_back(static_cast<uint16_t>(std::distance(m_state.m_symbols.begin(), it)), name_val.second);
        }
    }

    Value& VM::operator[](const std::string& name) noexcept
    {
        ExecutionContext& context = *m_execution_contexts.front();

        // const std::lock_guard<std::mutex> lock(m_mutex);

        // find id of object
        auto it = std::find(m_state.m_symbols.begin(), m_state.m_symbols.end(), name);
        if (it == m_state.m_symbols.end())
        {
            m_no_value = Builtins::nil;
            return m_no_value;
        }

        uint16_t id = static_cast<uint16_t>(std::distance(m_state.m_symbols.begin(), it));
        Value* var = findNearestVariable(id, context);
        if (var != nullptr)
            return *var;
        m_no_value = Builtins::nil;
        return m_no_value;
    }

    void VM::loadPlugin(uint16_t id, ExecutionContext& context)
    {
        namespace fs = std::filesystem;

        const std::string file = m_state.m_constants[id].stringRef().toString();

        std::string path = file;
        // bytecode loaded from file
        if (m_state.m_filename != ARK_NO_NAME_FILE)
            path = (fs::path(m_state.m_filename).parent_path() / fs::path(file)).relative_path().string();

        std::shared_ptr<SharedLibrary> lib;
        for (auto const& v : m_state.m_libenv)
        {
            std::string lib_path = (fs::path(v) / fs::path(file)).string();

            // if it's already loaded don't do anything
            if (std::find_if(m_shared_lib_objects.begin(), m_shared_lib_objects.end(), [&](const auto& val) {
                    return (val->path() == path || val->path() == lib_path);
                }) != m_shared_lib_objects.end())
                return;

            // if it exists alongside the .arkc file
            if (Utils::fileExists(path))
            {
                lib = std::make_shared<SharedLibrary>(path);
                break;
            }
            // check in lib_path otherwise
            else if (Utils::fileExists(lib_path))
            {
                lib = std::make_shared<SharedLibrary>(lib_path);
                break;
            }
        }

        if (!lib)
        {
            auto lib_path = std::accumulate(
                std::next(m_state.m_libenv.begin()),
                m_state.m_libenv.end(),
                m_state.m_libenv[0],
                [](const std::string& a, const std::string& b) -> std::string {
                    return a + "\n\t- " + b;
                });
            throwVMError("Could not find module '" + file + "'. Searched in\n\t- " + path + "\n\t- " + lib_path);
        }

        m_shared_lib_objects.emplace_back(lib);

        // load the mapping from the dynamic library
        mapping* map = nullptr;
        try
        {
            map = m_shared_lib_objects.back()->template get<mapping* (*)()>("getFunctionsMapping")();
        }
        catch (const std::system_error& e)
        {
            throwVMError(
                "An error occurred while loading module '" + file + "': " + std::string(e.what()) + "\n" +
                "It is most likely because the versions of the module and the language don't match.");
        }

        // load the mapping data
        std::size_t i = 0;
        while (map[i].name != nullptr)
        {
            // put it in the global frame, aka the first one
            auto it = std::find(m_state.m_symbols.begin(), m_state.m_symbols.end(), std::string(map[i].name));
            if (it != m_state.m_symbols.end())
                (*context.locals[0]).push_back(static_cast<uint16_t>(std::distance(m_state.m_symbols.begin(), it)), Value(map[i].value));

            // free memory because we have used it and don't need it anymore
            // no need to free map[i].value since it's a pointer to a function in the DLL
            delete[] map[i].name;
            ++i;
        }

        // free memory
        delete[] map;
    }

    void VM::exit(int code) noexcept
    {
        m_exit_code = code;
        m_running = false;
    }

    // ------------------------------------------
    //               user pointer
    // ------------------------------------------

    void VM::setUserPointer(void* ptr) noexcept
    {
        m_user_pointer = ptr;
    }

    void* VM::getUserPointer() noexcept
    {
        return m_user_pointer;
    }

    ExecutionContext* VM::createAndGetContext()
    {
        const std::lock_guard<std::mutex> lock(m_mutex);

        m_execution_contexts.push_back(std::make_unique<ExecutionContext>());
        ExecutionContext* ctx = m_execution_contexts.back().get();
        ctx->scope_count_to_delete.emplace_back(0);

        ctx->locals.reserve(m_execution_contexts.front()->locals.size());
        for (std::size_t i = 0, end = m_execution_contexts.front()->locals.size(); i < end; ++i)
        {
            ctx->locals.push_back(
                std::make_shared<Scope>(*m_execution_contexts.front()->locals[i]));
        }

        return ctx;
    }

    void VM::deleteContext(ExecutionContext* ec)
    {
        const std::lock_guard<std::mutex> lock(m_mutex);

        auto it = std::remove_if(
            m_execution_contexts.begin(),
            m_execution_contexts.end(),
            [ec](const std::unique_ptr<ExecutionContext>& ctx) {
                return ctx.get() == ec;
            });
        m_execution_contexts.erase(it);
    }

    Future* VM::createFuture(std::vector<Value>& args)
    {
        ExecutionContext* ctx = createAndGetContext();

        // doing this after having created the context
        // because the context uses the mutex and we don't want a deadlock
        const std::lock_guard<std::mutex> lock(m_mutex);
        m_futures.push_back(std::make_unique<Future>(ctx, this, args));

        return m_futures.back().get();
    }

    void VM::deleteFuture(Future* f)
    {
        const std::lock_guard<std::mutex> lock(m_mutex);

        auto it = std::remove_if(
            m_futures.begin(),
            m_futures.end(),
            [f](const std::unique_ptr<Future>& future) {
                return future.get() == f;
            });
        m_futures.erase(it);
    }

    // ------------------------------------------
    //                 execution
    // ------------------------------------------

    int VM::run() noexcept
    {
        init();
        safeRun(*m_execution_contexts[0]);

        // reset VM after each run
        for (auto& context : m_execution_contexts)
        {
            context->ip = 0;
            context->pp = 0;
        }

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
                uint8_t inst = m_state.m_pages[context.pp][context.ip];

                // and it's time to du-du-du-du-duel!
                switch (inst)
                {
#pragma region "Instructions"

                    case Instruction::LOAD_SYMBOL:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Load a symbol from its id onto the stack
                        */

                        ++context.ip;
                        context.last_symbol = readNumber(context);

                        if (Value* var = findNearestVariable(context.last_symbol, context); var != nullptr)
                            // push internal reference, shouldn't break anything so far
                            push(var, context);
                        else
                            throwVMError("unbound variable: " + m_state.m_symbols[context.last_symbol]);

                        COZ_PROGRESS_NAMED("ark vm load_symbol");
                        break;
                    }

                    case Instruction::LOAD_CONST:
                    {
                        /*
                            Argument: constant id (two bytes, big endian)
                            Job: Load a constant from its id onto the stack. Should check for a saved environment
                                    and push a Closure with the page address + environment instead of the constant
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        if (context.saved_scope && m_state.m_constants[id].valueType() == ValueType::PageAddr)
                        {
                            push(Value(Closure(context.saved_scope.value(), m_state.m_constants[id].pageAddr())), context);
                            context.saved_scope.reset();
                        }
                        else
                        {
                            // push internal ref
                            push(&(m_state.m_constants[id]), context);
                        }

                        COZ_PROGRESS_NAMED("ark vm load_const");
                        break;
                    }

                    case Instruction::POP_JUMP_IF_TRUE:
                    {
                        /*
                            Argument: absolute address to jump to (two bytes, big endian)
                            Job: Jump to the provided address if the last value on the stack was equal to true.
                                    Remove the value from the stack no matter what it is
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        if (*popAndResolveAsPtr(context) == Builtins::trueSym)
                            context.ip = static_cast<int16_t>(id) - 1;  // because we are doing a ++context.ip right after this
                        break;
                    }

                    case Instruction::STORE:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Take the value on top of the stack and put it inside a variable named following
                                    the symbol id (cf symbols table), in the nearest scope. Raise an error if it
                                    couldn't find a scope where the variable exists
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        if (Value* var = findNearestVariable(id, context); var != nullptr)
                        {
                            if (var->isConst())
                                throwVMError("can not modify a constant: " + m_state.m_symbols[id]);

                            *var = *popAndResolveAsPtr(context);
                            var->setConst(false);
                            break;
                        }

                        COZ_PROGRESS_NAMED("ark vm store");

                        throwVMError("unbound variable " + m_state.m_symbols[id] + ", can not change its value");
                        break;
                    }

                    case Instruction::LET:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Take the value on top of the stack and create a constant in the current scope, named
                                    following the given symbol id (cf symbols table)
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        // check if we are redefining a variable
                        if (auto val = (*context.locals.back())[id]; val != nullptr)
                            throwVMError("can not use 'let' to redefine the variable " + m_state.m_symbols[id]);

                        Value val = *popAndResolveAsPtr(context);
                        val.setConst(true);
                        (*context.locals.back()).push_back(id, val);

                        COZ_PROGRESS_NAMED("ark vm let");
                        break;
                    }

                    case Instruction::POP_JUMP_IF_FALSE:
                    {
                        /*
                            Argument: absolute address to jump to (two bytes, big endian)
                            Job: Jump to the provided address if the last value on the stack was equal to false. Remove
                                    the value from the stack no matter what it is
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        if (*popAndResolveAsPtr(context) == Builtins::falseSym)
                            context.ip = static_cast<int16_t>(id) - 1;  // because we are doing a ++context.ip right after this
                        break;
                    }

                    case Instruction::JUMP:
                    {
                        /*
                            Argument: absolute address to jump to (two byte, big endian)
                            Job: Jump to the provided address
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        context.ip = static_cast<int16_t>(id) - 1;  // because we are doing a ++context.ip right after this
                        break;
                    }

                    case Instruction::RET:
                    {
                        /*
                            Argument: none
                            Job: If in a code segment other than the main one, quit it, and push the value on top of
                                    the stack to the new stack ; should as well delete the current environment.
                        */

                        Value ip_or_val = *popAndResolveAsPtr(context);
                        // no return value on the stack
                        if (ip_or_val.valueType() == ValueType::InstPtr)
                        {
                            context.ip = ip_or_val.pageAddr();
                            // we always push PP then IP, thus the next value
                            // MUST be the page pointer
                            context.pp = pop(context)->pageAddr();

                            returnFromFuncCall(context);
                            push(Builtins::nil, context);
                        }
                        // value on the stack
                        else
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

                        COZ_PROGRESS_NAMED("ark vm ret");
                        break;
                    }

                    case Instruction::HALT:
                        m_running = false;
                        break;

                    case Instruction::CALL:
                        call(context);
                        break;

                    case Instruction::CAPTURE:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Used to tell the Virtual Machine to capture the variable from the current environment.
                                Main goal is to be able to handle closures, which need to save the environment in which
                                they were created
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        if (!context.saved_scope)
                            context.saved_scope = std::make_shared<Scope>();
                        // if it's a captured variable, it can not be nullptr
                        Value* ptr = (*context.locals.back())[id];
                        ptr = ptr->valueType() == ValueType::Reference ? ptr->reference() : ptr;
                        (*context.saved_scope.value()).push_back(id, *ptr);

                        COZ_PROGRESS_NAMED("ark vm capture");
                        break;
                    }

                    case Instruction::BUILTIN:
                    {
                        /*
                            Argument: id of builtin (two bytes, big endian)
                            Job: Push the builtin function object on the stack
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        push(Builtins::builtins[id].second, context);

                        COZ_PROGRESS_NAMED("ark vm builtin");
                        break;
                    }

                    case Instruction::MUT:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Take the value on top of the stack and create a variable in the current scope,
                                named following the given symbol id (cf symbols table)
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        Value val = *popAndResolveAsPtr(context);
                        val.setConst(false);

                        // avoid adding the pair (id, _) multiple times, with different values
                        Value* local = (*context.locals.back())[id];
                        if (local == nullptr)
                            (*context.locals.back()).push_back(id, val);
                        else
                            *local = val;

                        COZ_PROGRESS_NAMED("ark vm mut");
                        break;
                    }

                    case Instruction::DEL:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Remove a variable/constant named following the given symbol id (cf symbols table)
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        if (Value* var = findNearestVariable(id, context); var != nullptr)
                        {
                            if (var->valueType() == ValueType::User)
                                var->usertypeRef().del();
                            *var = Value();
                            break;
                        }

                        COZ_PROGRESS_NAMED("ark vm del");

                        throwVMError("unbound variable: " + m_state.m_symbols[id]);
                        break;
                    }

                    case Instruction::SAVE_ENV:
                    {
                        /*
                            Argument: none
                            Job: Save the current environment, useful for quoted code
                        */
                        context.saved_scope = context.locals.back();

                        COZ_PROGRESS_NAMED("ark vm save_scope");
                        break;
                    }

                    case Instruction::GET_FIELD:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Used to read the field named following the given symbol id (cf symbols table) of a `Closure`
                                stored in TS. Pop TS and push the value of field read on the stack
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        Value* var = popAndResolveAsPtr(context);
                        if (var->valueType() != ValueType::Closure)
                            throwVMError("the variable `" + m_state.m_symbols[context.last_symbol] + "' isn't a closure, can not get the field `" + m_state.m_symbols[id] + "' from it");

                        if (Value* field = (*var->refClosure().scope())[id]; field != nullptr)
                        {
                            // check for CALL instruction
                            if (static_cast<std::size_t>(context.ip) + 1 < m_state.m_pages[context.pp].size() && m_state.m_pages[context.pp][context.ip + 1] == Instruction::CALL)
                            {
                                context.locals.push_back(var->refClosure().scope());
                                ++context.scope_count_to_delete.back();
                            }

                            push(field, context);
                            break;
                        }

                        throwVMError("couldn't find the variable " + m_state.m_symbols[id] + " in the closure enviroment");
                        break;
                    }

                    case Instruction::PLUGIN:
                    {
                        /*
                            Argument: constant id (two bytes, big endian)
                            Job: Load a module named following the constant id (cf constants table).
                                 Raise an error if it couldn't find the module.
                        */

                        ++context.ip;
                        uint16_t id = readNumber(context);

                        loadPlugin(id, context);

                        COZ_PROGRESS_NAMED("ark vm plugin");
                        break;
                    }

                    case Instruction::LIST:
                    {
                        /*
                            Takes at least 0 arguments and push a list on the stack.
                            The content is pushed in reverse order
                        */
                        ++context.ip;
                        uint16_t count = readNumber(context);

                        Value l(ValueType::List);
                        if (count != 0)
                            l.list().reserve(count);

                        for (uint16_t i = 0; i < count; ++i)
                            l.push_back(*popAndResolveAsPtr(context));
                        push(std::move(l), context);

                        COZ_PROGRESS_NAMED("ark vm list");
                        break;
                    }

                    case Instruction::APPEND:
                    {
                        ++context.ip;
                        uint16_t count = readNumber(context);

                        Value* list = popAndResolveAsPtr(context);
                        if (list->valueType() != ValueType::List)
                            types::generateError(
                                "append",
                                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                                { *list });

                        const uint16_t size = list->constList().size();

                        Value obj = Value(*list);
                        obj.list().reserve(size + count);

                        for (uint16_t i = 0; i < count; ++i)
                            obj.push_back(*popAndResolveAsPtr(context));
                        push(std::move(obj), context);

                        COZ_PROGRESS_NAMED("ark vm append");
                        break;
                    }

                    case Instruction::CONCAT:
                    {
                        ++context.ip;
                        uint16_t count = readNumber(context);

                        Value* list = popAndResolveAsPtr(context);
                        if (list->valueType() != ValueType::List)
                            types::generateError(
                                "concat",
                                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                                { *list });

                        Value obj = Value(*list);

                        for (uint16_t i = 0; i < count; ++i)
                        {
                            Value* next = popAndResolveAsPtr(context);

                            if (list->valueType() != ValueType::List || next->valueType() != ValueType::List)
                                types::generateError(
                                    "concat",
                                    { { types::Contract { { types::Typedef("dst", ValueType::List), types::Typedef("src", ValueType::List) } } } },
                                    { *list, *next });

                            for (auto it = next->list().begin(), end = next->list().end(); it != end; ++it)
                                obj.push_back(*it);
                        }
                        push(std::move(obj), context);

                        COZ_PROGRESS_NAMED("ark vm concat");
                        break;
                    }

                    case Instruction::APPEND_IN_PLACE:
                    {
                        ++context.ip;
                        uint16_t count = readNumber(context);

                        Value* list = popAndResolveAsPtr(context);

                        if (list->isConst())
                            throwVMError("can not modify a constant list using `append!'");
                        if (list->valueType() != ValueType::List)
                            types::generateError(
                                "append!",
                                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                                { *list });

                        for (uint16_t i = 0; i < count; ++i)
                            list->push_back(*popAndResolveAsPtr(context));

                        push(Nil, context);

                        COZ_PROGRESS_NAMED("ark vm append!");
                        break;
                    }

                    case Instruction::CONCAT_IN_PLACE:
                    {
                        ++context.ip;
                        uint16_t count = readNumber(context);

                        Value* list = popAndResolveAsPtr(context);

                        if (list->isConst())
                            throwVMError("can not modify a constant list using `concat!'");
                        if (list->valueType() != ValueType::List)
                            types::generateError(
                                "concat",
                                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                                { *list });

                        for (uint16_t i = 0; i < count; ++i)
                        {
                            Value* next = popAndResolveAsPtr(context);

                            if (list->valueType() != ValueType::List || next->valueType() != ValueType::List)
                                types::generateError(
                                    "concat!",
                                    { { types::Contract { { types::Typedef("dst", ValueType::List), types::Typedef("src", ValueType::List) } } } },
                                    { *list, *next });

                            for (auto it = next->list().begin(), end = next->list().end(); it != end; ++it)
                                list->push_back(*it);
                        }

                        push(Nil, context);

                        COZ_PROGRESS_NAMED("ark vm concat!");
                        break;
                    }

                    case Instruction::POP_LIST:
                    {
                        Value list = *popAndResolveAsPtr(context);
                        Value number = *popAndResolveAsPtr(context);

                        if (list.valueType() != ValueType::List || number.valueType() != ValueType::Number)
                            types::generateError(
                                "pop",
                                { { types::Contract { { types::Typedef("list", ValueType::List), types::Typedef("index", ValueType::Number) } } } },
                                { list, number });

                        long idx = static_cast<long>(number.number());
                        idx = (idx < 0 ? list.list().size() + idx : idx);
                        if (static_cast<std::size_t>(idx) >= list.list().size())
                            throw std::runtime_error("pop: index out of range");

                        list.list().erase(list.list().begin() + idx);
                        push(list, context);
                        break;
                    }

                    case Instruction::POP_LIST_IN_PLACE:
                    {
                        Value* list = popAndResolveAsPtr(context);
                        Value number = *popAndResolveAsPtr(context);

                        if (list->isConst())
                            throwVMError("can not modify a constant list using `pop!'");
                        if (list->valueType() != ValueType::List || number.valueType() != ValueType::Number)
                            types::generateError(
                                "pop!",
                                { { types::Contract { { types::Typedef("list", ValueType::List), types::Typedef("index", ValueType::Number) } } } },
                                { *list, number });

                        long idx = static_cast<long>(number.number());
                        idx = (idx < 0 ? list->list().size() + idx : idx);
                        if (static_cast<std::size_t>(idx) >= list->list().size())
                            throw std::runtime_error("pop!: index out of range");

                        list->list().erase(list->list().begin() + idx);
                        break;
                    }

                    case Instruction::POP:
                    {
                        pop(context);
                        break;
                    }

#pragma endregion

#pragma region "Operators"

                    case Instruction::ADD:
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

                    case Instruction::SUB:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            types::generateError(
                                "-",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });
                        else
                            push(Value(a->number() - b->number()), context);
                        break;
                    }

                    case Instruction::MUL:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            types::generateError(
                                "*",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });
                        else
                            push(Value(a->number() * b->number()), context);
                        break;
                    }

                    case Instruction::DIV:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            types::generateError(
                                "/",
                                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                                { *a, *b });
                        else
                        {
                            auto d = b->number();
                            if (d == 0)
                                throw ZeroDivisionError();

                            push(Value(a->number() / d), context);
                        }
                        break;
                    }

                    case Instruction::GT:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push((!(*a == *b) && !(*a < *b)) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case Instruction::LT:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push((*a < *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case Instruction::LE:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push((((*a < *b) || (*a == *b)) ? Builtins::trueSym : Builtins::falseSym), context);
                        break;
                    }

                    case Instruction::GE:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push(!(*a < *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case Instruction::NEQ:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push((*a != *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case Instruction::EQ:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        push((*a == *b) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case Instruction::LEN:
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

                    case Instruction::EMPTY:
                    {
                        Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() == ValueType::List)
                            push((a->constList().size() == 0) ? Builtins::trueSym : Builtins::falseSym, context);
                        else if (a->valueType() == ValueType::String)
                            push((a->string().size() == 0) ? Builtins::trueSym : Builtins::falseSym, context);
                        else
                            types::generateError(
                                "empty?",
                                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                { *a });
                        break;
                    }

                    case Instruction::TAIL:
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
                                Value b = *a;
                                b.stringRef().erase_front(0);
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

                    case Instruction::HEAD:
                    {
                        Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() == ValueType::List)
                        {
                            if (a->constList().size() == 0)
                                push(Builtins::nil, context);
                            else
                            {
                                Value b = a->constList()[0];
                                push(b, context);
                            }
                        }
                        else if (a->valueType() == ValueType::String)
                        {
                            if (a->string().size() == 0)
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

                    case Instruction::ISNIL:
                    {
                        Value* a = popAndResolveAsPtr(context);
                        push((*a == Builtins::nil) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case Instruction::ASSERT:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (b->valueType() != ValueType::String)
                            types::generateError(
                                "assert",
                                { { types::Contract { { types::Typedef("expr", ValueType::Any), types::Typedef("message", ValueType::String) } } } },
                                { *a, *b });

                        if (*a == Builtins::falseSym)
                            throw AssertionFailed(b->stringRef().toString());
                        break;
                    }

                    case Instruction::TO_NUM:
                    {
                        Value* a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::String)
                            types::generateError(
                                "toNumber",
                                { { types::Contract { { types::Typedef("value", ValueType::String) } } } },
                                { *a });

                        double val;
                        if (Utils::isDouble(a->string().c_str(), &val))
                            push(Value(val), context);
                        else
                            push(Builtins::nil, context);
                        break;
                    }

                    case Instruction::TO_STR:
                    {
                        std::stringstream ss;
                        Value* a = popAndResolveAsPtr(context);
                        a->toString(ss, *this);
                        push(Value(ss.str()), context);
                        break;
                    }

                    case Instruction::AT:
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
                            push(a.list()[idx < 0 ? a.list().size() + idx : idx], context);
                        else if (a.valueType() == ValueType::String)
                            push(Value(std::string(1, a.string()[idx < 0 ? a.string().size() + idx : idx])), context);
                        else
                            types::generateError(
                                "@",
                                { { types::Contract { { types::Typedef("src", ValueType::List), types::Typedef("idx", ValueType::Number) } },
                                    types::Contract { { types::Typedef("src", ValueType::String), types::Typedef("idx", ValueType::Number) } } } },
                                { a, *b });
                        break;
                    }

                    case Instruction::AND_:
                    {
                        Value *a = popAndResolveAsPtr(context), *b = popAndResolveAsPtr(context);

                        push((*a == Builtins::trueSym && *b == Builtins::trueSym) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case Instruction::OR_:
                    {
                        Value *a = popAndResolveAsPtr(context), *b = popAndResolveAsPtr(context);

                        push((*b == Builtins::trueSym || *a == Builtins::trueSym) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

                    case Instruction::MOD:
                    {
                        Value *b = popAndResolveAsPtr(context), *a = popAndResolveAsPtr(context);

                        if (a->valueType() != ValueType::Number)
                            throw TypeError("Arguments of mod should be Numbers");
                        if (b->valueType() != ValueType::Number)
                            throw TypeError("Arguments of mod should be Numbers");

                        push(Value(std::fmod(a->number(), b->number())), context);
                        break;
                    }

                    case Instruction::TYPE:
                    {
                        Value* a = popAndResolveAsPtr(context);

                        push(Value(types_to_str[static_cast<unsigned>(a->valueType())]), context);
                        break;
                    }

                    case Instruction::HASFIELD:
                    {
                        Value *field = popAndResolveAsPtr(context), *closure = popAndResolveAsPtr(context);

                        if (closure->valueType() != ValueType::Closure)
                            throw TypeError("Argument no 1 of hasField should be a Closure");
                        if (field->valueType() != ValueType::String)
                            throw TypeError("Argument no 2 of hasField should be a String");

                        auto it = std::find(m_state.m_symbols.begin(), m_state.m_symbols.end(), field->stringRef().toString());
                        if (it == m_state.m_symbols.end())
                        {
                            push(Builtins::falseSym, context);
                            break;
                        }

                        uint16_t id = static_cast<uint16_t>(std::distance(m_state.m_symbols.begin(), it));
                        push((*closure->refClosure().refScope())[id] != nullptr ? Builtins::trueSym : Builtins::falseSym, context);

                        break;
                    }

                    case Instruction::NOT:
                    {
                        Value* a = popAndResolveAsPtr(context);

                        push(!(*a) ? Builtins::trueSym : Builtins::falseSym, context);
                        break;
                    }

#pragma endregion

                    default:
                        throwVMError("unknown instruction: " + std::to_string(static_cast<std::size_t>(inst)));
                        break;
                }

                // move forward
                ++context.ip;

#ifdef ARK_PROFILER_MIPS
                ++instructions_executed;
#endif
            }
        }
        catch (const std::exception& e)
        {
            std::printf("%s\n", e.what());
            backtrace(context);
            m_exit_code = 1;
        }
        catch (...)
        {
            std::printf("Unknown error\n");
            backtrace(context);
            m_exit_code = 1;
        }

        if (m_state.m_debug_level > 0)
            std::cout << "Estimated stack trashing: " << context.sp << "/" << VMStackSize << "\n";

#ifdef ARK_PROFILER_MIPS
        auto end_time = std::chrono::system_clock::now();
        auto d = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        std::cout << "\nInstructions executed: " << instructions_executed << "\n"
                  << "Time spent: " << d.count() << " us\n"
                  << (static_cast<double>(instructions_executed) / d.count()) << " MIPS\n";
#endif

        return m_exit_code;
    }

    // ------------------------------------------
    //             error handling
    // ------------------------------------------

    uint16_t VM::findNearestVariableIdWithValue(const Value& value, ExecutionContext& context) const noexcept
    {
        for (auto it = context.locals.rbegin(), it_end = context.locals.rend(); it != it_end; ++it)
        {
            if (auto id = (*it)->idFromValue(value); id < m_state.m_symbols.size())
                return id;
        }
        return std::numeric_limits<uint16_t>::max();
    }

    void VM::throwVMError(const std::string& message)
    {
        throw std::runtime_error(message);
    }

    void VM::backtrace(ExecutionContext& context) noexcept
    {
        int saved_ip = context.ip;
        std::size_t saved_pp = context.pp;
        uint16_t saved_sp = context.sp;

        if (context.fc > 1)
        {
            // display call stack trace
            uint16_t it = context.fc;
            Scope old_scope = *context.locals.back().get();

            while (it != 0)
            {
                std::cerr << "[" << termcolor::cyan << it << termcolor::reset << "] ";
                if (context.pp != 0)
                {
                    uint16_t id = findNearestVariableIdWithValue(
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
                    --it;
                }
                else
                {
                    std::printf("In global scope\n");
                    break;
                }

                if (context.fc - it > 7)
                {
                    std::printf("...\n");
                    break;
                }
            }

            // display variables values in the current scope
            std::printf("\nCurrent scope variables values:\n");
            for (std::size_t i = 0, size = old_scope.size(); i < size; ++i)
            {
                std::cerr << termcolor::cyan << m_state.m_symbols[old_scope.m_data[i].first] << termcolor::reset
                          << " = ";
                old_scope.m_data[i].second.toString(std::cerr, *this);
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
                  << "At IP: " << (saved_ip != -1 ? saved_ip : 0)
                  << ", PP: " << saved_pp
                  << ", SP: " << saved_sp
                  << "\n";
    }
}
