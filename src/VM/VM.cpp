#include <Ark/VM/VM.hpp>

// read a number from the bytecode
#define readNumber(var) {                                                \
    var = (static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip]) << 8) +   \
           static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip + 1]);      \
    ++m_ip;                                                              \
    }
// register a variable in the current scope
#define registerVariable(id, value) ((*m_locals.back()).push_back(id, value))
// register a variable in the global scope
#define registerVarGlobal(id, value) ((*m_locals[0]).push_back(id, value))
// stack management
#define popVal() m_frames.back().pop()
#define popValFrom(page) m_frames[static_cast<std::size_t>(page)].pop()
#define push(value) m_frames.back().push(value)
// create a new locals scope
#define createNewScope() m_locals.emplace_back(std::make_shared<Scope>());
// get a variable from a scope
#define getVariableInCurrentScope(id) (*m_locals.back())[id]

namespace Ark
{
    VM::VM(State* state) noexcept :
        m_state(state), m_ip(0), m_pp(0), m_running(false),
        m_last_sym_loaded(0), m_until_frame_count(0),
        m_user_pointer(nullptr)
    {
        m_frames.reserve(16);
        m_locals.reserve(4);
    }

    void VM::init() noexcept
    {
        using namespace Ark::internal;

        // clearing frames and setting up a new one
        if ((m_state->m_options & FeaturePersist) == 0)
        {
            m_frames.clear();
            m_frames.emplace_back();
            m_shared_lib_objects.clear();
        }
        else if (m_frames.size() == 0)
        {
            // if persistance is set but no frames are present, add one
            // it usually happens on the first run
            m_frames.emplace_back();
        }

        m_saved_scope.reset();

        // clearing locals (scopes) and create a global scope
        if ((m_state->m_options & FeaturePersist) == 0)
        {
            m_locals.clear();
            createNewScope();
        }
        else if (m_locals.size() == 0)
        {
            // if persistance is set but not scopes are present, add one
            createNewScope();
        }

        // loading binded functions
        // put them in the global frame if we can, aka the first one
        for (auto name_func : m_state->m_binded_functions)
        {
            auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), name_func.first);
            if (it != m_state->m_symbols.end())
                registerVarGlobal(static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it)), Value(name_func.second));
        }
    }

    internal::Value& VM::operator[](const std::string& name) noexcept
    {
        using namespace Ark::internal;

        const std::lock_guard<std::mutex> lock(m_mutex);

        // find id of object
        auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), name);
        if (it == m_state->m_symbols.end())
        {
            m__no_value = Builtins::nil;
            return m__no_value;
        }

        uint16_t id = static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it));
        Value* var = findNearestVariable(id);
        if (var != nullptr)
            return *var;
        m__no_value = Builtins::nil;
        return m__no_value;
    }

    void VM::loadPlugin(uint16_t id)
    {
        using namespace Ark::internal;
        namespace fs = std::filesystem;

        std::string file = m_state->m_constants[id].string_ref().toString();
        std::string path = "./" + file;

        if (m_state->m_filename != ARK_NO_NAME_FILE)  // bytecode loaded from file
            path = "./" + (fs::path(m_state->m_filename).parent_path() / fs::path(file)).string();
        std::string lib_path = (fs::path(m_state->m_libdir) / fs::path(file)).string();

        if (std::find_if(m_shared_lib_objects.begin(), m_shared_lib_objects.end(), [&, this](const auto& val) {
            return (val->path() == path || val->path() == lib_path);
        }) != m_shared_lib_objects.end())
            return;

        if (Utils::fileExists(path))  // if it exists alongside the .arkc file
            m_shared_lib_objects.emplace_back(std::make_shared<SharedLibrary>(path));
        else if (Utils::fileExists(lib_path))  // check in LOAD_PATH otherwise
            m_shared_lib_objects.emplace_back(std::make_shared<SharedLibrary>(lib_path));
        else
            throwVMError("could not load plugin: " + file);

        // load data from it!
        using Mapping_t = std::unordered_map<std::string, Value::ProcType>;
        using map_fun_t = Mapping_t(*) ();
        Mapping_t map;

        try {
            map = m_shared_lib_objects.back()->template get<map_fun_t>("getFunctionsMapping")();
        } catch (const std::system_error& e) {
            throwVMError(std::string(e.what()));
        }

        for (auto&& kv : map)
        {
            // put it in the global frame, aka the first one
            auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), kv.first);
            if (it != m_state->m_symbols.end())
                registerVarGlobal(static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it)), Value(kv.second));
        }
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
        using namespace Ark::internal;

        init();
        int out = safeRun();

        // reset VM after each run
        m_ip = 0;
        m_pp = 0;

        return out;
    }

    int VM::safeRun(std::size_t untilFrameCount)
    {
        using namespace Ark::internal;
        m_until_frame_count = untilFrameCount;

        try {
            m_running = true;
            while (m_running && m_frames.size() > m_until_frame_count)
            {
                // get current instruction
                uint8_t inst = m_state->m_pages[m_pp][m_ip];

                // and it's time to du-du-du-du-duel!
                switch (inst)
                {
                    case Instruction::LOAD_SYMBOL:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Load a symbol from its id onto the stack
                        */

                        ++m_ip;
                        readNumber(m_last_sym_loaded);

                        if (Value* var = findNearestVariable(m_last_sym_loaded); var != nullptr)
                            push(*var);
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
                        uint16_t id; readNumber(id);

                        if (m_saved_scope && m_state->m_constants[id].valueType() == ValueType::PageAddr)
                        {
                            push(Value(Closure(m_saved_scope.value(), m_state->m_constants[id].pageAddr())));
                            m_saved_scope.reset();
                        }
                        else
                            push(m_state->m_constants[id]);

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
                        uint16_t id; readNumber(id);

                        if (*popVal() == Builtins::trueSym)
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
                        uint16_t id; readNumber(id);

                        if (Value* var = findNearestVariable(id); var != nullptr)
                        {
                            if (var->isConst())
                                throwVMError("can not modify a constant: " + m_state->m_symbols[id]);
                            *var = *popVal();
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
                        uint16_t id; readNumber(id);

                        // check if we are redefining a variable
                        if (auto val = getVariableInCurrentScope(id); val != nullptr)
                            throwVMError("can not use 'let' to redefine the variable " + m_state->m_symbols[id]);

                        Value* val = popVal();
                        val->setConst(true);
                        registerVariable(id, *val);

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
                        uint16_t id; readNumber(id);

                        if (*popVal() == Builtins::falseSym)
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
                        uint16_t id; readNumber(id);

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

                        // save pp
                        PageAddr_t old_pp = static_cast<PageAddr_t>(m_pp);
                        m_pp = static_cast<std::size_t>(m_frames.back().callerPageAddr());
                        m_ip = static_cast<int>(m_frames.back().callerAddr());

                        Value return_value = m_frames.back().stackSize() != 0 ? *popVal() : Builtins::nil;
                        returnFromFuncCall();
                        push(return_value);

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
                        uint16_t id; readNumber(id);

                        if (!m_saved_scope)
                            m_saved_scope = std::make_shared<Scope>();
                        // if it's a captured variable, it can not be nullptr
                        (*m_saved_scope.value()).push_back(id, *getVariableInCurrentScope(id));

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
                        uint16_t id; readNumber(id);

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
                        uint16_t id; readNumber(id);

                        Value* val = popVal();
                        val->setConst(false);
                        registerVariable(id, *val);

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
                        uint16_t id; readNumber(id);

                        if (Value* var = findNearestVariable(id); var != nullptr)
                        {
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
                        uint16_t id; readNumber(id);

                        Value* var = popVal();
                        if (var->valueType() != ValueType::Closure)
                            throwVMError("the variable `" + m_state->m_symbols[m_last_sym_loaded] + "' isn't a closure, can not get the field `" + m_state->m_symbols[id] + "' from it");

                        if (Value* field = (*var->closure_ref().scope())[id]; field != nullptr)
                        {
                            // check for CALL instruction
                            if (m_ip + 1 < m_state->m_pages[m_pp].size() && m_state->m_pages[m_pp][m_ip + 1] == Instruction::CALL)
                            {
                                m_locals.push_back(var->closure_ref().scope());
                                m_frames.back().incScopeCountToDelete();
                            }

                            push(*field);
                            break;
                        }

                        throwVMError("couldn't find the variable " + m_state->m_symbols[id] + " in the closure enviroment");
                        break;
                    }

                    case Instruction::PLUGIN:
                    {
                        /*
                            Argument: constant id (two bytes, big endian)
                            Job: Load a plugin named following the constant id (cf constants table).
                                 Raise an error if it couldn't find the plugin.
                        */

                        ++m_ip;
                        uint16_t id; readNumber(id);

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
                        uint16_t count; readNumber(count);

                        Value l(ValueType::List);
                        if (count != 0)
                            l.list().reserve(count);

                        for (uint16_t i=0; i < count; ++i)
                            l.push_back(*popVal());
                        push(l);

                        COZ_PROGRESS_NAMED("ark vm list");
                        break;
                    }

                    case Instruction::APPEND:
                    {
                        ++m_ip;
                        uint16_t count; readNumber(count);

                        Value *list = popVal();
                        if (list->valueType() != ValueType::List)
                            throw Ark::TypeError("append needs a list and then whatever you want");
                        const uint16_t size = list->const_list().size();
                        list->list().reserve(size + count);

                        for (uint16_t i=0; i < count; ++i)
                            list->push_back(*popVal());
                        push(*list);

                        COZ_PROGRESS_NAMED("ark vm append");
                        break;
                    }

                    case Instruction::CONCAT:
                    {
                        ++m_ip;
                        uint16_t count; readNumber(count);

                        Value *list = popVal();
                        if (list->valueType() != ValueType::List)
                            throw Ark::TypeError("concat needs lists");

                        for (uint16_t i=0; i < count; ++i)
                        {
                            Value *next = popVal();
                            if (next->valueType() != ValueType::List)
                                throw Ark::TypeError("concat needs lists");

                            for (auto it=next->list().begin(), end=next->list().end(); it != end; ++it)
                                list->push_back(*it);
                        }
                        push(*list);

                        COZ_PROGRESS_NAMED("ark vm concat");
                        break;
                    }

                    case Instruction::ADD:
                    {
                        Value *b = popVal(), *a = popVal();
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
                        Value *b = popVal(), *a = popVal();
                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            throw Ark::TypeError("Arguments of - should be Numbers");

                        push(Value(a->number() - b->number()));
                        break;
                    }

                    case Instruction::MUL:
                    {
                        Value *b = popVal(), *a = popVal();
                        if (a->valueType() != ValueType::Number || b->valueType() != ValueType::Number)
                            throw Ark::TypeError("Arguments of * should be Numbers");

                        push(Value(a->number() * b->number()));
                        break;
                    }

                    case Instruction::DIV:
                    {
                        Value *b = popVal(), *a = popVal();
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
                        Value *b = popVal(), *a = popVal();
                        push((!(*a == *b) && !(*a < *b)) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::LT:
                    {
                        Value *b = popVal(), *a = popVal();
                        push((*a < *b) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::LE:
                    {
                        Value *b = popVal(), *a = popVal();
                        push(((*a < *b) || (*a == *b)) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::GE:
                    {
                        Value *b = popVal(), *a = popVal();
                        push(!(*a < *b) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::NEQ:
                    {
                        Value *b = popVal(), *a = popVal();
                        push((*a != *b) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::EQ:
                    {
                        Value *b = popVal(), *a = popVal();
                        push((*a == *b) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::LEN:
                    {
                        Value *a = popVal();
                        if (a->valueType() == ValueType::List)
                            push(Value(static_cast<int>(a->const_list().size())));
                        else if (a->valueType() == ValueType::String)
                            push(Value(static_cast<int>(a->string().size())));
                        else
                            throw Ark::TypeError("Argument of len must be a list or a String");
                        break;
                    }

                    case Instruction::EMPTY:
                    {
                        Value* a = popVal();
                        if (a->valueType() == ValueType::List)
                            push((a->const_list().size() == 0) ? Builtins::trueSym : Builtins::falseSym);
                        else if (a->valueType() == ValueType::String)
                            push((a->string().size() == 0) ? Builtins::trueSym : Builtins::falseSym);
                        else
                            throw Ark::TypeError("Argument of empty? must be a list or a String");

                        break;
                    }

                    case Instruction::FIRSTOF:
                    {
                        Value a = *popVal();
                        if (a.valueType() == ValueType::List)
                            push(a.const_list().size() > 0 ? (a.const_list())[0] : std::vector<Value>());
                        else if (a.valueType() == ValueType::String)
                            push(a.string().size() > 0 ? Value(std::string(1, (a.string())[0])) : "");
                        else
                            throw Ark::TypeError("Argument of firstOf must be a list");

                        break;
                    }

                    case Instruction::TAILOF:
                    {
                        Value* a = popVal();
                        if (a->valueType() == ValueType::List)
                        {
                            if (a->const_list().size() < 2)
                            {
                                push(std::vector<Value>());
                                break;
                            }

                            a->list().erase(a->const_list().begin());
                            push(*a);
                        }
                        else if (a->valueType() == ValueType::String)
                        {
                            if (a->string().size() < 2)
                            {
                                push("");
                                break;
                            }

                            a->string_ref().erase_front(0);
                            push(*a);
                        }
                        else
                            throw Ark::TypeError("Argument of tailOf must be a list or a String");

                        break;
                    }

                    case Instruction::HEADOF:
                    {
                        Value* a = popVal();
                        if (a->valueType() == ValueType::List)
                        {
                            if (a->const_list().size() < 2)
                            {
                                push(std::vector<Value>());
                                break;
                            }

                            a->list().pop_back();
                            push(*a);
                        }
                        else if (a->valueType() == ValueType::String)
                        {
                            if (a->string().size() < 2)
                            {
                                push("");
                                break;
                            }

                            a->string_ref().erase(a->string_ref().size() - 1);
                            push(*a);
                        }
                        else
                            throw Ark::TypeError("Argument of headOf must be a list or a String");

                        break;
                    }

                    case Instruction::ISNIL:
                    {
                        push((*popVal() == Builtins::nil) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::ASSERT:
                    {
                        Value *b = popVal(), *a = popVal();
                        if (*a == Builtins::falseSym)
                        {
                            if (b->valueType() != ValueType::String)
                                throw Ark::TypeError("Second argument of assert must be a String");

                            throw Ark::AssertionFailed(b->string_ref().toString());
                        }
                        break;
                    }

                    case Instruction::TO_NUM:
                    {
                        Value* a = popVal();
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
                        ss << (*popVal());
                        push(Value(ss.str()));
                        break;
                    }

                    case Instruction::AT:
                    {
                        Value *b = popVal(), a = *popVal();
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
                        Value *a = popVal(), *b = popVal();
                        push((*a == Builtins::trueSym && *b == Builtins::trueSym) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::OR_:
                    {
                        Value *a = popVal(), *b = popVal();
                        push((*b == Builtins::trueSym || *a == Builtins::trueSym) ? Builtins::trueSym : Builtins::falseSym);
                        break;
                    }

                    case Instruction::MOD:
                    {
                        Value *b = popVal(), *a = popVal();
                        if (a->valueType() != ValueType::Number)
                            throw Ark::TypeError("Arguments of mod should be Numbers");
                        if (b->valueType() != ValueType::Number)
                            throw Ark::TypeError("Arguments of mod should be Numbers");

                        push(Value(std::fmod(a->number(), b->number())));
                        break;
                    }

                    case Instruction::TYPE:
                    {
                        Value *a = popVal();
                        push(Value(types_to_str[static_cast<unsigned>(a->valueType())]));
                        break;
                    }

                    case Instruction::HASFIELD:
                    {
                        Value *field = popVal(), *closure = popVal();
                        if (closure->valueType() != ValueType::Closure)
                            throw Ark::TypeError("Argument no 1 of hasField should be a Closure");
                        if (field->valueType() != ValueType::String)
                            throw Ark::TypeError("Argument no 2 of hasField should be a String");

                        auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), field->string_ref().toString());
                        if (it == m_state->m_symbols.end())
                        {
                            push(Builtins::falseSym);
                            break;
                        }

                        uint16_t id = static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it));
                        push((*closure->closure_ref().scope_ref())[id] != nullptr ? Builtins::trueSym : Builtins::falseSym);

                        break;
                    }

                    case Instruction::NOT:
                    {
                        bool a = !(*popVal());
                        if (a)
                            push(Builtins::trueSym);
                        else
                            push(Builtins::falseSym);
                        break;
                    }

                    default:
                        throwVMError("unknown instruction: " + Ark::Utils::toString(static_cast<std::size_t>(inst)));
                        break;
                }

                // move forward
                ++m_ip;
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            backtrace();
            return 1;
        } catch (...) {
            std::cerr << "Unknown error" << std::endl;
            backtrace();
            return 1;
        }
        return 0;
    }

    // ------------------------------------------
    //             error handling
    // ------------------------------------------

    uint16_t VM::findNearestVariableIdWithValue(internal::Value&& value) noexcept
    {
        for (auto it=m_locals.rbegin(), it_end=m_locals.rend(); it != it_end; ++it)
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
        using namespace Ark::internal;
        std::cerr << termcolor::reset << "At IP: " << (m_ip != -1 ? m_ip : 0) << ", PP: " << m_pp << "\n";

        if (m_frames.size() > 1)
        {
            // display call stack trace
            for (auto&& it=m_frames.rbegin(), it_end=m_frames.rend(); it != it_end; ++it)
            {
                std::cerr << "[" << termcolor::cyan << std::distance(it, m_frames.rend()) << termcolor::reset << "] ";
                if (it->currentPageAddr() != 0)
                {
                    uint16_t id = findNearestVariableIdWithValue(
                        Value(static_cast<PageAddr_t>(it->currentPageAddr()))
                    );

                    if (id < m_state->m_symbols.size())
                        std::cerr << "In function `" << termcolor::green << m_state->m_symbols[id] << termcolor::reset << "'\n";
                    else  // should never happen
                        std::cerr << "In function `" << termcolor::green << "???" << termcolor::reset << "'\n";
                }
                else
                    std::cerr << "In global scope\n";

                if (std::distance(m_frames.rbegin(), it) > 7)
                {
                    std::cerr << "...\n";
                    break;
                }
            }

            // display variables values in the current scope
            std::cerr << "\nCurrent scope variables values:\n";
            for (std::size_t i=0, size=m_locals.back()->size(); i < size; ++i)
                std::cerr << termcolor::cyan << m_state->m_symbols[m_locals.back()->m_data[i].first] << termcolor::reset
                          << " = " << m_locals.back()->m_data[i].second << "\n";

            // if persistance is on, clear frames to keep only the global one
            if (m_state->m_options & FeaturePersist)
                m_frames.erase(m_frames.begin() + 1, m_frames.end());
        }
    }
}
