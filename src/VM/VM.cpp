#include <Ark/VM/VM.hpp>

// read a number from the bytecode
#define readNumber(var) {                                                        \
    auto x = (static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip]) << 8); ++m_ip; \
    auto y = static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip]);                \
    var = x + y;                                                                 \
    }
// register a variable in the current scope
#define registerVariable(id, value) ((*m_locals.back())[id] = value)
// register a variable in the global scope
#define registerVarGlobal(id, value) ((*m_locals[0])[id] = value)
// stack management
#define popVal() m_frames.back().pop()
#define popValFrom(page) m_frames[static_cast<std::size_t>(page)].pop()
#define push(value) m_frames.back().push(value)
// create a new locals scope
#define createNewScope() m_locals.emplace_back(std::make_shared<std::vector<Value>>(m_state->m_symbols.size(), ValueType::Undefined));
// get a variable from a scope
#define getVariableInCurrentScope(id) (*m_locals.back())[id]

namespace Ark
{
    VM::VM(State* state) :
        m_state(state), m_ip(0), m_pp(0), m_running(false), m_last_sym_loaded(0), m_until_frame_count(0)
    {
        m_frames.reserve(16);
        m_locals.reserve(4);
    }

    void VM::init()
    {
        using namespace Ark::internal;

        // clearing frames and setting up a new one
        if ((m_state->m_options & FeaturePersist) == 0)
        {
            m_frames.clear();
            m_frames.emplace_back();
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

        // loading plugins
        for (auto&& plugin : m_state->m_shared_lib_objects)
        {
            // load data from it!
            using Mapping_t = std::unordered_map<std::string, Value::ProcType>;
            using map_fun_t = Mapping_t(*) ();
            Mapping_t map = plugin.template get<map_fun_t>("getFunctionsMapping")();

            for (auto&& kv : map)
            {
                // put it in the global frame, aka the first one
                auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), kv.first);
                if (it != m_state->m_symbols.end())
                    registerVarGlobal(static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it)), Value(kv.second));
            }
        }
    }

    internal::Value& VM::operator[](const std::string& name)
    {
        using namespace Ark::internal;

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
        else
        {
            m__no_value = Builtins::nil;
            return m__no_value;
        }
    }

    // ------------------------------------------
    //                 execution
    // ------------------------------------------

    int VM::run()
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

        static const Value types_to_str[] = {
            Value("List"), Value("Number"), Value("String"), Value("Function"),
            Value("CProc"), Value("Closure"), Value("UserType"),
            Value("Nil"), Value("Bool"), Value("Bool"), Value("Undefined")
        };

        try {
            m_running = true;
            while (m_running && m_frames.size() > m_until_frame_count)
            {
                // get current instruction
                uint8_t inst = m_state->m_pages[m_pp][m_ip];

                // and it's time to du-du-du-du-duel!
                if (Instruction::FIRST_COMMAND <= inst && inst <= Instruction::LAST_COMMAND)
                    switch (inst)
                    {
                    case Instruction::LOAD_SYMBOL:
                    {
                        /*
                            Argument: symbol id (two bytes, big endian)
                            Job: Load a symbol from its id onto the stack
                        */

                        ++m_ip;
                        uint16_t id; readNumber(id);

                        Value* var = findNearestVariable(id);
                        if (var != nullptr)
                        {
                            push(*var);
                            m_last_sym_loaded = id;
                            break;
                        }

                        throwVMError("couldn't find symbol to load: " + m_state->m_symbols[id]);
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

                        if (m_saved_scope && m_state->m_constants[id].m_type == ValueType::PageAddr)
                        {
                            push(Value(Closure(m_saved_scope.value(), m_state->m_constants[id].pageAddr())));
                            m_saved_scope.reset();
                        }
                        else
                            push(m_state->m_constants[id]);
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
                        int16_t addr = static_cast<int16_t>(id);

                        if (popVal()->m_type == ValueType::True)
                            m_ip = addr - 1;  // because we are doing a ++m_ip right after this
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

                        Value* var = findNearestVariable(id);
                        if (var != nullptr)
                        {
                            if (var->m_const)
                                throwVMError("can not modify a constant: " + m_state->m_symbols[id]);
                            *var = *popVal();
                            break;
                        }

                        throwVMError("couldn't find symbol: " + m_state->m_symbols[id]);
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
                        if (getVariableInCurrentScope(id).m_type != ValueType::Undefined)
                            throwVMError("can not use 'let' to redefine the variable " + m_state->m_symbols[id]);

                        registerVariable(id, *popVal()).m_const = true;
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
                        int16_t addr = static_cast<int16_t>(id);

                        if (popVal()->m_type == ValueType::False)
                            m_ip = addr - 1;  // because we are doing a ++m_ip right after this
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
                        int16_t addr = static_cast<int16_t>(id);

                        m_ip = addr - 1;  // because we are doing a ++m_ip right after this
                        break;
                    }
                    
                    case Instruction::RET:
                    {
                        /*
                            Argument: none
                            Job: If in a code segment other than the main one, quit it, and push the value on top of
                                    the stack to the new stack ; should as well delete the current environment.
                                    Otherwise, acts as a `HALT`
                        */

                        // check if we should halt the VM
                        if (m_pp == 0)
                        {
                            m_running = false;
                            break;
                        }

                        // save pp
                        PageAddr_t old_pp = static_cast<PageAddr_t>(m_pp);
                        m_pp = static_cast<std::size_t>(m_frames.back().callerPageAddr());
                        m_ip = static_cast<int>(m_frames.back().callerAddr());

                        if (m_frames.back().stackSize() != 0)
                        {
                            Value return_value = *popVal();
                            returnFromFuncCall();
                            // push value as the return value of a function to the current stack
                            push(return_value);
                        }
                        else
                        {
                            returnFromFuncCall();
                            push(Builtins::nil);
                        }
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
                        {
                            m_saved_scope = std::make_shared<std::vector<Value>>(
                                m_state->m_symbols.size(), ValueType::Undefined
                            );
                        }
                        (*m_saved_scope.value())[id] = getVariableInCurrentScope(id);
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

                        registerVariable(id, *popVal()).m_const = false;
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

                        Value* var = findNearestVariable(id);
                        if (var != nullptr)
                        {
                            *var = Value(ValueType::Undefined);
                            break;
                        }

                        throwVMError("couldn't find symbol: " + m_state->m_symbols[id]);
                        break;
                    }

                    case Instruction::SAVE_ENV:
                    {
                        /*
                            Argument: none
                            Job: Save the current environment, useful for quoted code
                        */
                        m_saved_scope = m_locals.back();
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
                        if (var->m_type != ValueType::Closure)
                            throwVMError("variable `" + m_state->m_symbols[m_last_sym_loaded] + "' isn't a closure, can not get the field `" + m_state->m_symbols[id] + "' from it");
                        
                        const Value& field = (*var->closure_ref().scope())[id];
                        if (field.m_type != ValueType::Undefined)
                        {
                            // check for CALL instruction
                            if (m_ip + 1 < m_state->m_pages[m_pp].size() && m_state->m_pages[m_pp][m_ip + 1] == Instruction::CALL)
                            {
                                m_locals.push_back(var->closure_ref().scope());
                                m_frames.back().incScopeCountToDelete();
                            }

                            push(field);
                            break;
                        }

                        throwVMError("couldn't find symbol in closure enviroment: " + m_state->m_symbols[id]);
                        break;
                    }
                    
                    default:
                        throwVMError("unknown instruction: " + Ark::Utils::toString(static_cast<std::size_t>(inst)));
                        break;
                    }
                else if (Instruction::FIRST_OPERATOR <= inst && inst <= Instruction::LAST_OPERATOR)
                    switch (inst)
                    {
                        case Instruction::ADD:
                        {
                            Value *b = popVal(), *a = popVal();
                            if (a->m_type == ValueType::Number)
                            {
                                if (b->m_type != ValueType::Number)
                                    throw Ark::TypeError("Arguments of + should have the same type");

                                push(Value(a->number() + b->number()));
                                break;
                            }
                            else if (a->m_type == ValueType::String)
                            {
                                if (b->m_type != ValueType::String)
                                    throw Ark::TypeError("Arguments of + should have the same type");

                                push(Value(a->string() + b->string()));
                                break;
                            }
                            throw Ark::TypeError("Arguments of + should be Numbers or Strings");
                        }

                        case Instruction::SUB:
                        {
                            Value *b = popVal(), *a = popVal();
                            if (a->m_type != ValueType::Number)
                                throw Ark::TypeError("Arguments of - should be Numbers");
                            if (b->m_type != ValueType::Number)
                                throw Ark::TypeError("Arguments of - should be Numbers");

                            push(Value(a->number() - b->number()));
                            break;
                        }

                        case Instruction::MUL:
                        {
                            Value *b = popVal(), *a = popVal();
                            if (a->m_type != ValueType::Number)
                                throw Ark::TypeError("Arguments of * should be Numbers");
                            if (b->m_type != ValueType::Number)
                                throw Ark::TypeError("Arguments of * should be Numbers");

                            push(Value(a->number() * b->number()));
                            break;
                        }

                        case Instruction::DIV:
                        {
                            Value *b = popVal(), *a = popVal();
                            if (a->m_type != ValueType::Number)
                                throw Ark::TypeError("Arguments of / should be Numbers");
                            if (b->m_type != ValueType::Number)
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
                            if (a->m_type == ValueType::List)
                            {
                                push(Value(static_cast<int>(a->const_list().size())));
                                break;
                            }
                            if (a->m_type == ValueType::String)
                            {
                                push(Value(static_cast<int>(a->string().size())));
                                break;
                            }

                            throw Ark::TypeError("Argument of len must be a list or a String");
                        }

                        case Instruction::EMPTY:
                        {
                            Value* a = popVal();
                            if (a->m_type == ValueType::List)
                                push((a->const_list().size() == 0) ? Builtins::trueSym : Builtins::falseSym);
                            else if (a->m_type == ValueType::String)
                                push((a->string().size() == 0) ? Builtins::trueSym : Builtins::falseSym);
                            else
                                throw Ark::TypeError("Argument of empty? must be a list or a String");
                            
                            break;
                        }

                        case Instruction::FIRSTOF:
                        {
                            Value a = *popVal();
                            if (a.m_type == ValueType::List)
                                push(a.const_list().size() > 0 ? (a.const_list())[0] : Builtins::nil);
                            else if (a.m_type == ValueType::String)
                                push(a.string().size() > 0 ? Value(std::string(1, (a.string())[0])) : Builtins::nil);
                            else
                                throw Ark::TypeError("Argument of firstOf must be a list");

                            break;
                        }

                        case Instruction::TAILOF:
                        {
                            Value* a = popVal();
                            if (a->m_type == ValueType::List)
                            {
                                if (a->const_list().size() < 2)
                                {
                                    push(Builtins::nil);
                                    break;
                                }

                                a->list().erase(a->const_list().begin());
                                push(*a);
                            }
                            else if (a->m_type == ValueType::String)
                            {
                                if (a->string().size() < 2)
                                {
                                    push(Builtins::nil);
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
                            if (a->m_type == ValueType::List)
                            {
                                if (a->const_list().size() < 2)
                                {
                                    push(Builtins::nil);
                                    break;
                                }

                                a->list().pop_back();
                                push(*a);
                            }
                            else if (a->m_type == ValueType::String)
                            {
                                if (a->string().size() < 2)
                                {
                                    push(Builtins::nil);
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
                                if (b->m_type != ValueType::String)
                                    throw Ark::TypeError("Second argument of assert must be a String");

                                throw Ark::AssertionFailed(b->string_ref().toString());
                            }
                            break;
                        }

                        case Instruction::TO_NUM:
                        {
                            Value* a = popVal();
                            if (a->m_type != ValueType::String)
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
                            if (b->m_type != ValueType::Number)
                                throw Ark::TypeError("Argument 2 of @ should be a Number");

                            if (a.m_type == ValueType::List)
                                push(a.list()[static_cast<long>(b->number())]);
                            else if (a.m_type == ValueType::String)
                                push(Value(std::string(1, a.string()[static_cast<long>(b->number())])));
                            else
                                throw Ark::TypeError("Argument 1 of @ should be a List or a String");
                            break;
                        }

                        case Instruction::AND_:
                        {
                            Value *a = popVal(), *b = popVal();
                            push((a->m_type == ValueType::True && b->m_type == ValueType::True) ? Builtins::trueSym : Builtins::falseSym);
                            break;
                        }

                        case Instruction::OR_:
                        {
                            Value *a = popVal(), *b = popVal();
                            push((b->m_type == ValueType::True || a->m_type == ValueType::True) ? Builtins::trueSym : Builtins::falseSym);
                            break;
                        }

                        case Instruction::MOD:
                        {
                            Value *b = popVal(), *a = popVal();
                            if (a->m_type != ValueType::Number)
                                throw Ark::TypeError("Arguments of mod should be Numbers");
                            if (b->m_type != ValueType::Number)
                                throw Ark::TypeError("Arguments of mod should be Numbers");
                            
                            push(Value(std::fmod(a->number(), b->number())));
                            break;
                        }

                        case Instruction::TYPE:
                        {
                            Value *a = popVal();
                            push(types_to_str[static_cast<unsigned>(a->m_type)]);
                            break;
                        }

                        case Instruction::HASFIELD:
                        {
                            Value *field = popVal(), *closure = popVal();
                            if (closure->m_type != ValueType::Closure)
                                throw Ark::TypeError("Argument no 1 of hasField should be a Closure");
                            if (field->m_type != ValueType::String)
                                throw Ark::TypeError("Argument no 2 of hasField should be a String");

                            auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), field->string_ref().toString());
                            if (it == m_state->m_symbols.end())
                            {
                                push(Builtins::falseSym);
                                break;
                            }
                            uint16_t id = static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it));

                            if ((*closure->closure_ref().scope_ref())[id].m_type != ValueType::Undefined)
                                push(Builtins::trueSym);
                            else
                                push(Builtins::falseSym);

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
                    }
                else
                    throwVMError("unknown instruction: " + Ark::Utils::toString(static_cast<std::size_t>(inst)));

                // move forward
                ++m_ip;
            }
        } catch (const std::exception& e) {
            std::cerr << "\n" << termcolor::red << e.what() << "\n";
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

    uint16_t VM::findNearestVariableIdWithValue(internal::Value&& value)
    {
        for (auto it=m_locals.rbegin(), it_end=m_locals.rend(); it != it_end; ++it)
        {
            for (auto sub=(*it)->begin(), sub_end=(*it)->end(); sub != sub_end; ++sub)
            {
                if (*sub == value)
                    return static_cast<uint16_t>(std::distance((*it)->begin(), sub));
            }
        }
        // oversized by one: didn't find anything
        return static_cast<uint16_t>(m_state->m_symbols.size());
    }

    void VM::throwVMError(const std::string& message)
    {
        throw std::runtime_error("VMError: " + message);
    }

    void VM::backtrace()
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

                    std::cerr << "In function `" << termcolor::green << m_state->m_symbols[id] << termcolor::reset << "'\n";
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
            {
                if ((*m_locals.back())[i].m_type != ValueType::Undefined)
                    std::cerr << termcolor::cyan << m_state->m_symbols[i] << termcolor::reset << " = " << (*m_locals.back())[i] << "\n";
            }

            // if persistance is on, clear frames to keep only the global one
            if (m_state->m_options & FeaturePersist)
                m_frames.erase(m_frames.begin() + 1, m_frames.end());
        }
    }
}