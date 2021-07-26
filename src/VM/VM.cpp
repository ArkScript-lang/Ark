#include <Ark/VM/VM.hpp>

#include <termcolor.hpp>

struct mapping
{
    char* name;
    Ark::Value (*value)(std::vector<Ark::Value>&, Ark::VM*);
};

namespace Ark
{
    using namespace internal;

    VM::VM(State* state) noexcept :
        m_state(state), m_exit_code(0), m_ip(0), m_pp(0), m_sp(0), m_fc(0),
        m_running(false), m_last_sym_loaded(0),
        m_until_frame_count(0), m_stack(nullptr), m_user_pointer(nullptr)
    {
        m_locals.reserve(4);
    }

    void VM::init() noexcept
    {
        // initialize the stack
        if (m_stack == nullptr)
            m_stack = std::make_unique<std::array<Value, ArkVMStackSize>>();

        m_sp = 0;
        m_fc = 1;

        m_shared_lib_objects.clear();
        m_scope_count_to_delete.clear();
        m_scope_count_to_delete.emplace_back(0);

        m_saved_scope.reset();
        m_exit_code = 0;

        m_locals.clear();
        createNewScope();

        if (m_locals.size() == 0)
        {
            // if persistance is set but not scopes are present, add one
            createNewScope();
        }

        // loading binded stuff
        // put them in the global frame if we can, aka the first one
        for (auto name_val : m_state->m_binded)
        {
            auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), name_val.first);
            if (it != m_state->m_symbols.end())
                (*m_locals[0]).push_back(static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it)), name_val.second);
        }
    }

    Value& VM::operator[](const std::string& name) noexcept
    {
        const std::lock_guard<std::mutex> lock(m_mutex);

        // find id of object
        auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), name);
        if (it == m_state->m_symbols.end())
        {
            m_no_value = Builtins::nil;
            return m_no_value;
        }

        uint16_t id = static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it));
        Value* var = findNearestVariable(id);
        if (var != nullptr)
            return *var;
        m_no_value = Builtins::nil;
        return m_no_value;
    }

    void VM::loadPlugin(uint16_t id)
    {
        namespace fs = std::filesystem;

        const std::string file = m_state->m_constants[id].stringRef().toString();

        std::string path = file;
        // bytecode loaded from file
        if (m_state->m_filename != ARK_NO_NAME_FILE)
            path = (fs::path(m_state->m_filename).parent_path() / fs::path(file)).relative_path().string();

        std::string lib_path = (fs::path(m_state->m_libdir) / fs::path(file)).string();

        // if it's already loaded don't do anything
        if (std::find_if(m_shared_lib_objects.begin(), m_shared_lib_objects.end(), [&, this](const auto& val) {
            return (val->path() == path || val->path() == lib_path);
        }) != m_shared_lib_objects.end())
            return;

        // if it exists alongside the .arkc file
        if (Utils::fileExists(path))
            m_shared_lib_objects.emplace_back(std::make_shared<SharedLibrary>(path));
        // check in lib_path otherwise
        else if (Utils::fileExists(lib_path))
            m_shared_lib_objects.emplace_back(std::make_shared<SharedLibrary>(lib_path));
        else
            throwVMError("Could not find module '" + file + "'. Searched in\n\t- " + path + "\n\t- " + lib_path);

        // load the mapping from the dynamic library
        mapping* map;
        try
        {
            map = m_shared_lib_objects.back()->template get<mapping* (*)()>("getFunctionsMapping")();
        }
        catch (const std::system_error& e)
        {
            throwVMError(
                "An error occurred while loading module '" + file + "': " + std::string(e.what()) + "\n" +
                "It is most likely because the versions of the module and the language don't match."
            );
        }

        // load the mapping data
        std::size_t i = 0;
        while (map[i].name != nullptr)
        {
            // put it in the global frame, aka the first one
            auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), std::string(map[i].name));
            if (it != m_state->m_symbols.end())
                (*m_locals[0]).push_back(static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it)), Value(map[i].value));

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

    // ------------------------------------------
    //                 execution
    // ------------------------------------------

    int VM::run() noexcept
    {
        init();
        safeRun();

        // reset VM after each run
        m_ip = 0;
        m_pp = 0;

        return m_exit_code;
    }

    int VM::safeRun(std::size_t untilFrameCount)
    {
        m_until_frame_count = untilFrameCount;

        try
        {
            m_running = true;
            while (m_running && m_fc > m_until_frame_count)
            {
                // get current instruction
                uint8_t inst = m_state->m_pages[m_pp][m_ip];

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

                        ++m_ip;
                        m_last_sym_loaded = readNumber();

                        if (Value* var = findNearestVariable(m_last_sym_loaded); var != nullptr)
                            // push internal reference, shouldn't break anything so far
                            push(var);
                        else
                            throwVMError("unbound variable: " + m_state->m_symbols[m_last_sym_loaded]);

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

                        ++m_ip;
                        uint16_t id = readNumber();

                        if (m_saved_scope && m_state->m_constants[id].valueType() == ValueType::PageAddr)
                        {
                            push(Value(Closure(m_saved_scope.value(), m_state->m_constants[id].pageAddr())));
                            m_saved_scope.reset();
                        }
                        else
                        {
                            // push internal ref
                            push(&(m_state->m_constants[id]));
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

                        ++m_ip;
                        uint16_t id = readNumber();

                        if (*popAndResolveAsPtr() == Builtins::trueSym)
                            m_ip = static_cast<int16_t>(id) - 1;  // because we are doing a ++m_ip right after this
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

                        ++m_ip;
                        uint16_t id = readNumber();

                        if (Value* var = findNearestVariable(id); var != nullptr)
                        {
                            if (var->isConst())
                                throwVMError("can not modify a constant: " + m_state->m_symbols[id]);

                            *var = *popAndResolveAsPtr();
                            var->setConst(false);
                            break;
                        }

                        COZ_PROGRESS_NAMED("ark vm store");

                        throwVMError("unbound variable " + m_state->m_symbols[id] + ", can not change its value");
                        break;
                    }

                    case Instruction::LET:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Take the value on top of the stack and create a constant in the current scope, named
                                    following the given symbol id (cf symbols table)
                        */

                        ++m_ip;
                        uint16_t id = readNumber();

                        // check if we are redefining a variable
                        if (auto val = (*m_locals.back())[id]; val != nullptr)
                            throwVMError("can not use 'let' to redefine the variable " + m_state->m_symbols[id]);

                        Value val = *popAndResolveAsPtr();
                        val.setConst(true);
                        (*m_locals.back()).push_back(id, val);

                        COZ_PROGRESS("ark vm let");
                        break;
                    }

                    case Instruction::POP_JUMP_IF_FALSE:
                    {
                        /*
                            Argument: absolute address to jump to (two bytes, big endian)
                            Job: Jump to the provided address if the last value on the stack was equal to false. Remove
                                    the value from the stack no matter what it is
                        */

                        ++m_ip;
                        uint16_t id = readNumber();

                        if (*popAndResolveAsPtr() == Builtins::falseSym)
                            m_ip = static_cast<int16_t>(id) - 1;  // because we are doing a ++m_ip right after this
                        break;
                    }

                    case Instruction::JUMP:
                    {
                        /*
                            Argument: absolute address to jump to (two byte, big endian)
                            Job: Jump to the provided address
                        */

                        ++m_ip;
                        uint16_t id = readNumber();

                        m_ip = static_cast<int16_t>(id) - 1;  // because we are doing a ++m_ip right after this
                        break;
                    }

                    case Instruction::RET:
                    {
                        /*
                            Argument: none
                            Job: If in a code segment other than the main one, quit it, and push the value on top of
                                    the stack to the new stack ; should as well delete the current environment.
                        */

                        Value ip_or_val = *popAndResolveAsPtr();
                        // no return value on the stack
                        if (ip_or_val.valueType() == ValueType::InstPtr)
                        {
                            m_ip = ip_or_val.pageAddr();
                            // we always push PP then IP, thus the next value
                            // MUST be the page pointer
                            m_pp = pop()->pageAddr();

                            returnFromFuncCall();
                            push(Builtins::nil);
                        }
                        // value on the stack
                        else
                        {
                            Value* ip;
                            do
                            {
                                ip = popAndResolveAsPtr();
                            } while(ip->valueType() != ValueType::InstPtr);

                            m_ip = ip->pageAddr();
                            m_pp = pop()->pageAddr();

                            returnFromFuncCall();
                            push(std::move(ip_or_val));
                        }

                        COZ_PROGRESS_NAMED("ark vm ret");
                        break;
                    }

                    case Instruction::HALT:
                        m_running = false;
                        break;

                    case Instruction::CALL:
                        call();
                        break;

                    case Instruction::CAPTURE:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Used to tell the Virtual Machine to capture the variable from the current environment.
                                Main goal is to be able to handle closures, which need to save the environment in which
                                they were created
                        */

                        ++m_ip;
                        uint16_t id = readNumber();

                        if (!m_saved_scope)
                            m_saved_scope = std::make_shared<Scope>();
                        // if it's a captured variable, it can not be nullptr
                        Value* ptr = (*m_locals.back())[id];
                        ptr = ptr->valueType() == ValueType::Reference ? ptr->reference() : ptr;
                        (*m_saved_scope.value()).push_back(id, *ptr);

                        COZ_PROGRESS_NAMED("ark vm capture");
                        break;
                    }

                    case Instruction::BUILTIN:
                    {
                        /*
                            Argument: id of builtin (two bytes, big endian)
                            Job: Push the builtin function object on the stack
                        */

                        ++m_ip;
                        uint16_t id = readNumber();

                        push(Builtins::builtins[id].second);

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

                        ++m_ip;
                        uint16_t id = readNumber();

                        Value val = *popAndResolveAsPtr();
                        val.setConst(false);

                        // avoid adding the pair (id, _) multiple times, with different values
                        Value* local = (*m_locals.back())[id];
                        if (local == nullptr)
                            (*m_locals.back()).push_back(id, val);
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

                        ++m_ip;
                        uint16_t id = readNumber();

                        if (Value* var = findNearestVariable(id); var != nullptr)
                        {
                            // free usertypes
                            if (var->valueType() == ValueType::User)
                                var->usertypeRef().del();
                            *var = Value();
                            break;
                        }

                        COZ_PROGRESS_NAMED("ark vm del");

                        throwVMError("unbound variable: " + m_state->m_symbols[id]);
                        break;
                    }

                    case Instruction::SAVE_ENV:
                    {
                        /*
                            Argument: none
                            Job: Save the current environment, useful for quoted code
                        */
                        m_saved_scope = m_locals.back();

                        COZ_PROGRESS("ark vm save_scope");
                        break;
                    }

                    case Instruction::GET_FIELD:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Used to read the field named following the given symbol id (cf symbols table) of a `Closure`
                                stored in TS. Pop TS and push the value of field read on the stack
                        */

                        ++m_ip;
                        uint16_t id = readNumber();

                        Value* var = popAndResolveAsPtr();
                        if (var->valueType() != ValueType::Closure)
                            throwVMError("the variable `" + m_state->m_symbols[m_last_sym_loaded] + "' isn't a closure, can not get the field `" + m_state->m_symbols[id] + "' from it");

                        if (Value* field = (*var->refClosure().scope())[id]; field != nullptr)
                        {
                            // check for CALL instruction
                            if (m_ip + 1 < m_state->m_pages[m_pp].size() && m_state->m_pages[m_pp][m_ip + 1] == Instruction::CALL)
                            {
                                m_locals.push_back(var->refClosure().scope());
                                ++m_scope_count_to_delete.back();
                            }

                            push(field);
                            break;
                        }

                        throwVMError("couldn't find the variable " + m_state->m_symbols[id] + " in the closure enviroment");
                        break;
                    }

                    case Instruction::PLUGIN:
                    {
                        /*
                            Argument: constant id (two bytes, big endian)
                            Job: Load a module named following the constant id (cf constants table).
                                 Raise an error if it couldn't find the module.
                        */

                        ++m_ip;
                        uint16_t id = readNumber();

                        loadPlugin(id);

                        COZ_PROGRESS("ark vm plugin");
                        break;
                    }

                    case Instruction::LIST:
                    {
                        /*
                            Takes at least 0 arguments and push a list on the stack.
                            The content is pushed in reverse order
                        */
                        ++m_ip;
                        uint16_t count = readNumber();

                        Value l(ValueType::List);
                        if (count != 0)
                            l.list().reserve(count);

                        for (uint16_t i = 0; i < count; ++i)
                            l.push_back(*popAndResolveAsPtr());
                        push(std::move(l));

                        COZ_PROGRESS_NAMED("ark vm list");
                        break;
                    }

                    case Instruction::APPEND:
                    {
                        ++m_ip;
                        uint16_t count = readNumber();

                        Value* list = popAndResolveAsPtr();
                        if (list->valueType() != ValueType::List)
                            throw Ark::TypeError("append needs a List and then whatever you want");
                        const uint16_t size = list->constList().size();

                        Value obj = Value(*list);
                        obj.list().reserve(size + count);

                        for (uint16_t i = 0; i < count; ++i)
                            obj.push_back(*popAndResolveAsPtr());
                        push(std::move(obj));

                        COZ_PROGRESS_NAMED("ark vm append");
                        break;
                    }

                    case Instruction::CONCAT:
                    {
                        ++m_ip;
                        uint16_t count = readNumber();

                        Value *list = popAndResolveAsPtr();
                        if (list->valueType() != ValueType::List)
                            throw Ark::TypeError("concat needs lists, got " + types_to_str[static_cast<unsigned>(list->valueType())]);

                        Value obj = Value(*list);

                        for (uint16_t i = 0; i < count; ++i)
                        {
                            Value* next = popAndResolveAsPtr();
                            if (next->valueType() != ValueType::List)
                                throw Ark::TypeError("concat needs lists");

                            for (auto it = next->list().begin(), end = next->list().end(); it != end; ++it)
                                obj.push_back(*it);
                        }
                        push(std::move(obj));

                        COZ_PROGRESS_NAMED("ark vm concat");
                        break;
                    }

                #pragma endregion

                #pragma region "Operators"

                    case Instruction::ADD:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        if (a->valueType() == ValueType::Number)
                        {
                            if (b->valueType() != ValueType::Number)
                                throw Ark::TypeError("Arguments of + should have the same type");

                            push(Value(a->number() + b->number()));
                            break;
                        }
                        else if (a->valueType() == ValueType::String)
                        {
                            if (b->valueType() != ValueType::String)
                                throw Ark::TypeError("Arguments of + should have the same type");

                            push(Value(a->string() + b->string()));
                            break;
                        }
                        throw Ark::TypeError("Arguments of + should be Numbers or Strings");
                    }

                    case Instruction::SUB:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            throw Ark::TypeError("Arguments of - should be Numbers");

                        push(Value(a->number() - b->number()));
                        break;
                    }

                    case Instruction::MUL:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            throw Ark::TypeError("Arguments of * should be Numbers");

                        push(Value(a->number() * b->number()));
                        break;
                    }

                    case Instruction::DIV:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            throw Ark::TypeError("Arguments of / should be Numbers");

                        auto d = b->number();
                        if (d == 0)
                            throw Ark::ZeroDivisionError();

                        push(Value(a->number() / d));
                        break;
                    }

                    case Instruction::GT:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        push((!(*a == *b) && !(*a < *b)) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::LT:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        push((*a < *b) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::LE:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        push(((*a < *b) || (*a == *b)) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::GE:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        push(!(*a < *b) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::NEQ:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        push((*a != *b) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::EQ:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        push((*a == *b) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::LEN:
                    {
                        Value *a = popAndResolveAsPtr();

                        if (a->valueType() == ValueType::List)
                            push(Value(static_cast<int>(a->constList().size())));
                        else if (a->valueType() == ValueType::String)
                            push(Value(static_cast<int>(a->string().size())));
                        else
                            throw Ark::TypeError("Argument of len must be a List or a String");
                        break;
                    }

                    case Instruction::EMPTY:
                    {
                        Value* a = popAndResolveAsPtr();

                        if (a->valueType() == ValueType::List)
                            push((a->constList().size() == 0) ? Builtins::trueSym : Builtins::falseSym);
                        else if (a->valueType() == ValueType::String)
                            push((a->string().size() == 0) ? Builtins::trueSym : Builtins::falseSym);
                        else
                            throw Ark::TypeError("Argument of empty? must be a List or a String");

                        break;
                    }

                    case Instruction::TAIL:
                    {
                        Value* a = popAndResolveAsPtr();

                        if (a->valueType() == ValueType::List)
                        {
                            if (a->constList().size() < 2)
                            {
                                push(Value(ValueType::List));
                                break;
                            }

                            std::vector<Value> tmp(a->constList().size() - 1);
                            for (std::size_t i = 1, end = a->constList().size(); i < end; ++i)
                                tmp[i - 1] = a->constList()[i];
                            push(Value(std::move(tmp)));
                        }
                        else if (a->valueType() == ValueType::String)
                        {
                            if (a->string().size() < 2)
                            {
                                push(Value(ValueType::String));
                                break;
                            }

                            Value b = *a;
                            b.stringRef().erase_front(0);
                            push(std::move(b));
                        }
                        else
                            throw Ark::TypeError("Argument of tail must be a List or a String");

                        break;
                    }

                    case Instruction::HEAD:
                    {
                        Value* a = popAndResolveAsPtr();

                        if (a->valueType() == ValueType::List)
                        {
                            if (a->constList().size() == 0)
                            {
                                push(Builtins::nil);
                                break;
                            }

                            Value b = a->constList()[0];
                            push(b);
                        }
                        else if (a->valueType() == ValueType::String)
                        {
                            if (a->string().size() == 0)
                            {
                                push(Value(ValueType::String));
                                break;
                            }

                            push(Value(std::string(1, a->stringRef()[0])));
                        }
                        else
                            throw Ark::TypeError("Argument of head must be a List or a String");

                        break;
                    }

                    case Instruction::ISNIL:
                    {
                        Value* a = popAndResolveAsPtr();
                        push((*a == Builtins::nil) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::ASSERT:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        if (*a == Builtins::falseSym)
                        {
                            if (b->valueType() != ValueType::String)
                                throw Ark::TypeError("Second argument of assert must be a String");

                            throw Ark::AssertionFailed(b->stringRef().toString());
                        }
                        break;
                    }

                    case Instruction::TO_NUM:
                    {
                        Value* a = popAndResolveAsPtr();

                        if (a->valueType() != ValueType::String)
                            throw Ark::TypeError("Argument of toNumber must be a String");

                        double val;
                        if (Utils::isDouble(a->string().c_str(), &val))
                            push(Value(val));
                        else
                            push(Builtins::nil);
                        break;
                    }

                    case Instruction::TO_STR:
                    {
                        std::stringstream ss;
                        Value* a = popAndResolveAsPtr();
                        ss << (*a);
                        push(Value(ss.str()));
                        break;
                    }

                    case Instruction::AT:
                    {
                        Value *b = popAndResolveAsPtr();
                        Value a = *popAndResolveAsPtr();  // be careful, it's not a pointer

                        if (b->valueType() != ValueType::Number)
                            throw Ark::TypeError("Argument 2 of @ should be a Number");

                        long idx = static_cast<long>(b->number());

                        if (a.valueType() == ValueType::List)
                            push(a.list()[idx < 0 ? a.list().size() + idx : idx]);
                        else if (a.valueType() == ValueType::String)
                            push(Value(std::string(1, a.string()[idx < 0 ? a.string().size() + idx : idx])));
                        else
                            throw Ark::TypeError("Argument 1 of @ should be a List or a String");
                        break;
                    }

                    case Instruction::AND_:
                    {
                        Value *a = popAndResolveAsPtr(), *b = popAndResolveAsPtr();

                        push((*a == Builtins::trueSym && *b == Builtins::trueSym) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::OR_:
                    {
                        Value *a = popAndResolveAsPtr(), *b = popAndResolveAsPtr();

                        push((*b == Builtins::trueSym || *a == Builtins::trueSym) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::MOD:
                    {
                        Value *b = popAndResolveAsPtr(), *a = popAndResolveAsPtr();

                        if (a->valueType() != ValueType::Number)
                            throw Ark::TypeError("Arguments of mod should be Numbers");
                        if (b->valueType() != ValueType::Number)
                            throw Ark::TypeError("Arguments of mod should be Numbers");

                        push(Value(std::fmod(a->number(), b->number())));
                        break;
                    }

                    case Instruction::TYPE:
                    {
                        Value *a = popAndResolveAsPtr();

                        push(Value(types_to_str[static_cast<unsigned>(a->valueType())]));
                        break;
                    }

                    case Instruction::HASFIELD:
                    {
                        Value *field = popAndResolveAsPtr(), *closure = popAndResolveAsPtr();

                        if (closure->valueType() != ValueType::Closure)
                            throw Ark::TypeError("Argument no 1 of hasField should be a Closure");
                        if (field->valueType() != ValueType::String)
                            throw Ark::TypeError("Argument no 2 of hasField should be a String");

                        auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), field->stringRef().toString());
                        if (it == m_state->m_symbols.end())
                        {
                            push(Builtins::falseSym);
                            break;
                        }

                        uint16_t id = static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it));
                        push((*closure->refClosure().refScope())[id] != nullptr ? Builtins::trueSym : Builtins::falseSym);

                        break;
                    }

                    case Instruction::NOT:
                    {
                        Value* a = popAndResolveAsPtr();

                        push(!(*a) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                #pragma endregion

                    default:
                        throwVMError("unknown instruction: " + std::to_string(static_cast<std::size_t>(inst)));
                        break;
                }

                // move forward
                ++m_ip;
            }
        }
        catch (const std::exception& e)
        {
            std::printf("%s\n", e.what());
            backtrace();
            m_exit_code = 1;
        }
        catch (...)
        {
            std::printf("Unknown error\n");
            backtrace();
            m_exit_code = 1;
        }

        return m_exit_code;
    }

    // ------------------------------------------
    //             error handling
    // ------------------------------------------

    uint16_t VM::findNearestVariableIdWithValue(Value&& value) noexcept
    {
        for (auto it = m_locals.rbegin(), it_end = m_locals.rend(); it != it_end; ++it)
        {
            if (auto id = (*it)->idFromValue(std::move(value)); id < m_state->m_symbols.size())
                return id;
        }
        return static_cast<uint16_t>(~0);
    }

    void VM::throwVMError(const std::string& message)
    {
        throw std::runtime_error(message);
    }

    void VM::backtrace() noexcept
    {
        std::cerr << termcolor::reset
                  << "At IP: " << (m_ip != -1 ? m_ip : 0)
                  << ", PP: " << m_pp
                  << ", SP: " << m_sp
                  << "\n";

        if (m_fc > 1)
        {
            // display call stack trace
            uint16_t it = m_fc;
            Scope old_scope = *m_locals.back().get();

            while (it != 0)
            {
                std::cerr << "[" << termcolor::cyan << it << termcolor::reset << "] ";
                if (m_pp != 0)
                {
                    uint16_t id = findNearestVariableIdWithValue(
                        Value(static_cast<PageAddr_t>(m_pp))
                    );

                    if (id < m_state->m_symbols.size())
                        std::cerr << "In function `" << termcolor::green << m_state->m_symbols[id] << termcolor::reset << "'\n";
                    else  // should never happen
                        std::cerr << "In function `" << termcolor::yellow << "???" << termcolor::reset << "'\n";

                    Value* ip;
                    do
                    {
                        ip = popAndResolveAsPtr();
                    } while (ip->valueType() != ValueType::InstPtr);

                    m_ip = ip->pageAddr();
                    m_pp = pop()->pageAddr();
                    returnFromFuncCall();
                    --it;
                }
                else
                {
                    std::printf("In global scope\n");
                    break;
                }

                if (m_fc - it > 7)
                {
                    std::printf("...\n");
                    break;
                }
            }

            // display variables values in the current scope
            std::printf("\nCurrent scope variables values:\n");
            for (std::size_t i = 0, size = old_scope.size(); i < size; ++i)
                std::cerr << termcolor::cyan << m_state->m_symbols[old_scope.m_data[i].first] << termcolor::reset
                          << " = " << old_scope.m_data[i].second << "\n";

            while (m_fc != 1)
            {
                Value* tmp = pop();
                if (tmp->valueType() == ValueType::InstPtr)
                    --m_fc;
                else if (tmp->valueType() == ValueType::User)
                    tmp->usertypeRef().del();
            }
            // pop the PP as well
            pop();
        }
    }
}
