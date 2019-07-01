#include <Ark/VM/VM.hpp>

#include <exception>
#include <stdexcept>
#include <filesystem>
#include <cassert>

#include <Ark/Log.hpp>
#include <Ark/VM/FFI.hpp>
#include <Ark/Utils.hpp>
#undef abs
#include <cmath>

namespace Ark
{
    using namespace std::string_literals;
    using namespace Ark::internal;

    VM::VM(bool debug, bool count_fcall) :
        m_debug(debug),
        m_count_fcall(count_fcall),
        m_fcalls(0),
        m_ip(0), m_pp(0),
        m_running(false),
        m_filename("FILE")
    {}
    
    void VM::feed(const std::string& filename)
    {
        Ark::BytecodeReader bcr;
        bcr.feed(filename);
        m_bytecode = bcr.bytecode();

        m_filename = filename;

        configure();
    }

    void VM::feed(const bytecode_t& bytecode)
    {
        m_bytecode = bytecode;

        configure();
    }

    void VM::run()
    {
        // reset VM before each run
        m_ip = 0;
        m_pp = 0;
        m_frames.clear();
        m_saved_frame.reset();
        createNewFrame();

        // loading plugins
        for (const auto& file: m_plugins)
        {
            namespace fs = std::filesystem;

            std::string path = "./" + file;
            if (m_filename != "FILE")  // bytecode loaded from file
                path = "./" + (fs::path(m_filename).parent_path() / fs::path(file)).string();
            std::string lib_path = (fs::path(ARK_STD) / fs::path(file)).string();

            if (m_debug)
                Ark::logger.info("Loading", file, "in", path, "or in", lib_path);

            if (Ark::Utils::fileExists(path))  // if it exists alongside the .arkc file
                m_shared_lib_objects.emplace_back(path);
            else if (Ark::Utils::fileExists(lib_path))  // check in LOAD_PATH otherwise
                m_shared_lib_objects.emplace_back(lib_path);
            else
                throwVMError("could not load plugin " + file);

            // load data from it!
            using Mapping_t = std::unordered_map<std::string, Value::ProcType>;
            Mapping_t map = m_shared_lib_objects.back().get<Mapping_t (*) ()>("getFunctionsMapping")();

            for (auto&& kv : map)
            {
                // put it in the global frame, aka the first one
                auto it = std::find(m_symbols.begin(), m_symbols.end(), kv.first);
                if (it != m_symbols.end())
                {
                    if (m_debug)
                        Ark::logger.info("Loading", kv.first);

                    frontFrame()[static_cast<uint16_t>(std::distance(m_symbols.begin(), it))] = Value(kv.second);
                }
            }
        }

        if (m_pages.size() > 0)
        {
            if (m_debug)
                Ark::logger.info("Starting at PP:{0}, IP:{1}"s, m_pp, m_ip);

            try
            {
                m_running = true;
                while (m_running)
                {
                    if (m_pp >= m_pages.size())
                        throwVMError("page pointer has gone too far (" + Ark::Utils::toString(m_pp) + ")");
                    if (m_ip >= m_pages[m_pp].size())
                        throwVMError("instruction pointer has gone too far (" + Ark::Utils::toString(m_ip) + ")");

                    // get current instruction
                    uint8_t inst = m_pages[m_pp][m_ip];

                    // and it's time to du-du-du-du-duel!
                    if (Instruction::FIRST_INSTRUCTION <= inst && inst <= Instruction::LAST_INSTRUCTION)
                        switch (inst)
                        {
                        case Instruction::NOP:
                            nop();
                            break;
                        
                        case Instruction::LOAD_SYMBOL:
                            loadSymbol();
                            break;
                        
                        case Instruction::LOAD_CONST:
                            loadConst();
                            break;
                        
                        case Instruction::POP_JUMP_IF_TRUE:
                            popJumpIfTrue();
                            break;
                        
                        case Instruction::STORE:
                            store();
                            break;
                        
                        case Instruction::LET:
                            let();
                            break;
                        
                        case Instruction::POP_JUMP_IF_FALSE:
                            popJumpIfFalse();
                            break;
                        
                        case Instruction::JUMP:
                            jump();
                            break;
                        
                        case Instruction::RET:
                            ret();
                            break;
                        
                        case Instruction::HALT:
                            m_running = false;
                            break;
                        
                        case Instruction::CALL:
                            call();
                            break;
                        
                        case Instruction::SAVE_ENV:
                            saveEnv();
                            break;
                        
                        case Instruction::BUILTIN:
                            builtin();
                            break;
                        
                        case Instruction::MUT:
                            mut();
                            break;
                        
                        case Instruction::DEL:
                            del();
                            break;
                        
                        default:
                            throwVMError("unknown instruction: " + Ark::Utils::toString(static_cast<std::size_t>(inst)) +
                                ", pp: " +Ark::Utils::toString(m_pp) + ", ip: " + Ark::Utils::toString(m_ip)
                            );
                        }
                    else if (Instruction::FIRST_OPERATOR <= inst && inst <= Instruction::LAST_OPERATOR)
                        operators(inst);
                    
                    // move forward
                    ++m_ip;
                }
            }
            catch (const std::exception& e)
            {
                Ark::logger.error(e.what());
            }

            if (m_count_fcall)
                std::cout << "Function calls: " << m_fcalls << "\n";
        }
    }

    void VM::loadFunction(const std::string& name, Value::ProcType function)
    {
        // put it in the global frame, aka the first one
        auto it = std::find(m_symbols.begin(), m_symbols.end(), name);
        if (it == m_symbols.end())
        {
            if (m_debug)
                Ark::logger.warn("Couldn't find symbol with name", name, "to set its value as a function");
            return;
        }

        frontFrame()[static_cast<uint16_t>(std::distance(m_symbols.begin(), it))] = Value(function);
    }

    void VM::configure()
    {
        // configure tables and pages
        const bytecode_t& b = m_bytecode;
        std::size_t i = 0;

        auto readNumber = [&b] (std::size_t& i) -> uint16_t {
            uint16_t x = (static_cast<uint16_t>(b[  i]) << 8) +
                            static_cast<uint16_t>(b[++i]);
            return x;
        };

        // read tables and check if bytecode is valid
        if (!(b.size() > 4 && b[i++] == 'a' && b[i++] == 'r' && b[i++] == 'k' && b[i++] == Instruction::NOP))
            throwVMError("invalid format: couldn't find magic constant");

        if (m_debug)
            Ark::logger.info("(Virtual Machine) magic constant found: ark\\0");

        uint16_t major = readNumber(i); i++;
        uint16_t minor = readNumber(i); i++;
        uint16_t patch = readNumber(i); i++;

        if (m_debug)
            Ark::logger.info("(Virtual Machine) version used: ", major, ".", minor, ".", patch);
        
        if (major != ARK_VERSION_MAJOR)
        {
            std::string str_version = Ark::Utils::toString(major) + "." +
                Ark::Utils::toString(minor) + "." +
                Ark::Utils::toString(patch);
            std::string builtin_version = Ark::Utils::toString(ARK_VERSION_MAJOR) + "." +
                Ark::Utils::toString(ARK_VERSION_MINOR) + "." +
                Ark::Utils::toString(ARK_VERSION_PATCH);
            throwVMError("Compiler and VM versions don't match: " + str_version + " and " + builtin_version);
        }

        using timestamp_t = unsigned long long;
        timestamp_t timestamp = 
            (static_cast<timestamp_t>(m_bytecode[  i]) << 56) +
            (static_cast<timestamp_t>(m_bytecode[++i]) << 48) +
            (static_cast<timestamp_t>(m_bytecode[++i]) << 40) +
            (static_cast<timestamp_t>(m_bytecode[++i]) << 32) +
            (static_cast<timestamp_t>(m_bytecode[++i]) << 24) +
            (static_cast<timestamp_t>(m_bytecode[++i]) << 16) +
            (static_cast<timestamp_t>(m_bytecode[++i]) <<  8) +
            (static_cast<timestamp_t>(m_bytecode[++i]))
            ;
        ++i;

        if (m_debug)
            Ark::logger.info("(Virtual Machine) timestamp: ", timestamp);

        if (b[i] == Instruction::SYM_TABLE_START)
        {
            if (m_debug)
                Ark::logger.info("(Virtual Machine) symbols table");
            
            i++;
            uint16_t size = readNumber(i);
            m_symbols.reserve(size);
            i++;

            if (m_debug)
                Ark::logger.info("(Virtual Machine) length:", size);
            
            for (uint16_t j=0; j < size; ++j)
            {
                std::string symbol = "";
                while (b[i] != 0)
                    symbol.push_back(b[i++]);
                i++;

                m_symbols.push_back(symbol);

                if (m_debug)
                    Ark::logger.info("(Virtual Machine) -", symbol);
            }
        }
        else
            throwVMError("couldn't find symbols table");

        if (b[i] == Instruction::VAL_TABLE_START)
        {
            if (m_debug)
                Ark::logger.info("(Virtual Machine) constants table");
            
            i++;
            uint16_t size = readNumber(i);
            m_constants.reserve(size);
            i++;

            if (m_debug)
                Ark::logger.info("(Virtual Machine) length:", size);

            for (uint16_t j=0; j < size; ++j)
            {
                uint8_t type = b[i];
                i++;

                if (type == Instruction::NUMBER_TYPE)
                {
                    std::string val = "";
                    while (b[i] != 0)
                        val.push_back(b[i++]);
                    i++;

                    m_constants.emplace_back(std::stod(val));
                    
                    if (m_debug)
                        Ark::logger.info("(Virtual Machine) - (Number)", val);
                }
                else if (type == Instruction::STRING_TYPE)
                {
                    std::string val = "";
                    while (b[i] != 0)
                        val.push_back(b[i++]);
                    i++;

                    m_constants.emplace_back(val);
                    
                    if (m_debug)
                        Ark::logger.info("(Virtual Machine) - (String)", val);
                }
                else if (type == Instruction::FUNC_TYPE)
                {
                    uint16_t addr = readNumber(i);
                    i++;

                    m_constants.emplace_back(addr);

                    if (m_debug)
                        Ark::logger.info("(Virtual Machine) - (PageAddr)", addr);
                    
                    i++;  // skip NOP
                }
                else
                    throwVMError("unknown value type for value " + Ark::Utils::toString(j));
            }
        }
        else
            throwVMError("couldn't find constants table");

        if (b[i] == Instruction::PLUGIN_TABLE_START)
        {
            if (m_debug)
                Ark::logger.info("(Virtual Machine) plugins table");
            
            i++;
            uint16_t size = readNumber(i);
            m_plugins.reserve(size);
            i++;

            if (m_debug)
                Ark::logger.info("(Virtual Machine) length:", size);
            
            for (uint16_t j=0; j < size; ++j)
            {
                std::string plugin = "";
                while (b[i] != 0)
                    plugin.push_back(b[i++]);
                i++;

                m_plugins.push_back(plugin);

                if (m_debug)
                    Ark::logger.info("(Virtual Machine) -", plugin);
            }
        }
        else
            throwVMError("couldn't find plugins table");
        
        while (b[i] == Instruction::CODE_SEGMENT_START)
        {
            if (m_debug)
                Ark::logger.info("(Virtual Machine) code segment");
            
            i++;
            uint16_t size = readNumber(i);
            i++;

            if (m_debug)
                Ark::logger.info("(Virtual Machine) length:", size);
            
            m_pages.emplace_back();
            m_pages.back().reserve(size);

            for (uint16_t j=0; j < size; ++j)
                m_pages.back().push_back(b[i++]);
            
            if (i == b.size())
                break;
        }
    }

    Value VM::pop(int page)
    {
        if (page == -1)
            return m_frames.back()->pop();
        return m_frames[static_cast<std::size_t>(page)]->pop();
    }

    void VM::push(const Value& value)
    {
        m_frames.back()->push(value);
    }

    inline void VM::nop()
    {
        // Does nothing
        if (m_debug)
            Ark::logger.info("NOP PP:{0}, IP:{1}"s, m_pp, m_ip);
    }
    
    inline void VM::loadSymbol()
    {
        /*
            Argument: symbol id (two bytes, big endian)
            Job: Load a symbol from its id onto the stack
        */
        ++m_ip;
        auto id = readNumber();

        if (m_debug)
            Ark::logger.info("LOAD_SYMBOL ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

        for (auto it=m_frames.rbegin(); it != m_frames.rend(); ++it)
        {
            if ((*it)->find(id))
            {
                push((**it)[id]);
                return;
            }
        }

        throwVMError("couldn't find symbol to load: " + m_symbols[id]);
    }
    
    inline void VM::loadConst()
    {
        /*
            Argument: constant id (two bytes, big endian)
            Job: Load a constant from its id onto the stack. Should check for a saved environment
                    and push a Closure with the page address + environment instead of the constant
        */
        ++m_ip;
        auto id = readNumber();

        if (m_debug)
            Ark::logger.info("LOAD_CONST ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);
        
        if (m_saved_frame && m_constants[id].valueType() == ValueType::PageAddr)
        {
            push(Value(Closure(m_frames[m_saved_frame.value()], m_constants[id].pageAddr())));
            m_saved_frame.reset();
        }
        else
            push(m_constants[id]);
    }
    
    inline void VM::popJumpIfTrue()
    {
        /*
            Argument: absolute address to jump to (two bytes, big endian)
            Job: Jump to the provided address if the last value on the stack was equal to true.
                    Remove the value from the stack no matter what it is
        */
        ++m_ip;
        int16_t addr = static_cast<int16_t>(readNumber());

        if (m_debug)
            Ark::logger.info("POP_JUMP_IF_TRUE ({0}) PP:{1}, IP:{2}"s, addr, m_pp, m_ip);

        if (pop() == FFI::trueSym)
            m_ip = addr - 1;  // because we are doing a ++m_ip right after this
    }
    
    inline void VM::store()
    {
        /*
            Argument: symbol id (two bytes, big endian)
            Job: Take the value on top of the stack and put it inside a variable named following
                    the symbol id (cf symbols table), in the nearest scope. Raise an error if it
                    couldn't find a scope where the variable exists
        */
        ++m_ip;
        auto id = readNumber();

        if (m_debug)
            Ark::logger.info("STORE ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

        for (auto it=m_frames.rbegin(); it != m_frames.rend(); ++it)
        {
            if ((*it)->find(id) && !(**it)[id].isConst())
            {
                (**it)[id] = pop();
                if ((**it)[id].valueType() == ValueType::Closure)
                    (**it)[id].closure_ref().save(-std::distance(m_frames.rend(), it) - 1, id);
                return;
            }
        }

        throwVMError("couldn't find symbol: " + m_symbols[id]);
    }
    
    inline void VM::let()
    {
        /*
            Argument: symbol id (two bytes, big endian)
            Job: Take the value on top of the stack and create a constant in the current scope, named
                    following the given symbol id (cf symbols table)
        */
        ++m_ip;
        auto id = readNumber();

        if (m_debug)
            Ark::logger.info("LET ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);
        
        // check if we are redefining a variable
        if (backFrame().find(id))
            throwVMError("can not use 'let' to redefine a symbol");

        backFrame()[id] = pop();
        backFrame()[id].setConst(true);
        if (backFrame()[id].valueType() == ValueType::Closure)
            backFrame()[id].closure_ref().save(m_frames.size() - 1, id);
    }

    inline void VM::popJumpIfFalse()
    {
        /*
            Argument: absolute address to jump to (two bytes, big endian)
            Job: Jump to the provided address if the last value on the stack was equal to false. Remove
                    the value from the stack no matter what it is
        */
        ++m_ip;
        int16_t addr = static_cast<int16_t>(readNumber());

        if (m_debug)
            Ark::logger.info("POP_JUMP_IF_FALSE ({0}) PP:{1}, IP:{2}"s, addr, m_pp, m_ip);

        if (pop() == FFI::falseSym)
            m_ip = addr - 1;  // because we are doing a ++m_ip right after this
    }
    
    inline void VM::jump()
    {
        /*
            Argument: absolute address to jump to (two byte, big endian)
            Job: Jump to the provided address
        */
        ++m_ip;
        int16_t addr = static_cast<int16_t>(readNumber());

        if (m_debug)
            Ark::logger.info("JUMP ({0}) PP:{1}, IP:{2}"s, addr, m_pp, m_ip);

        m_ip = addr - 1;  // because we are doing a ++m_ip right after this
    }
    
    inline void VM::ret()
    {
        /*
            Argument: none
            Job: If in a code segment other than the main one, quit it, and push the value on top of
                    the stack to the new stack ; should as well delete the current environment.
                    Otherwise, acts as a `HALT`
        */
        if (m_debug)
            Ark::logger.info("RET PP:{0}, IP:{1}"s, m_pp, m_ip);
        
        // check if we should halt the VM
        if (m_pp == 0)
        {
            m_running = false;
            return;
        }

        // save pp
        PageAddr_t old_pp = static_cast<PageAddr_t>(m_pp);
        m_pp = m_frames.back()->callerPageAddr();
        m_ip = m_frames.back()->callerAddr();

        auto rm_frame = [this] () -> void {
            // remove frame
            m_frames.pop_back();
            // next frame is the one of the closure
            // remove it
            m_frames.pop_back();
        };
        
        if (m_frames.back()->stackSize() != 0)
        {
            Value return_value(pop());
            rm_frame();
            // push value as the return value of a function to the current stack
            push(return_value);
        }
        else
            rm_frame();
    }
    
    inline void VM::call()
    {
        /*
            Argument: number of arguments when calling the function
            Job: Call function from its symbol id located on top of the stack. Take the given number of
                    arguments from the top of stack and give them  to the function (the first argument taken
                    from the stack will be the last one of the function). The stack of the function is now composed
                    of its arguments, from the first to the last one
        */
        ++m_ip;
        auto argc = readNumber();

        if (m_debug)
            Ark::logger.info("CALL ({0}) PP:{1}, IP:{2}"s, argc, m_pp, m_ip);
        
        if (m_count_fcall)
            m_fcalls++;

        Value function(pop());

        switch (function.valueType())
        {
            // is it a builtin function name?
            case ValueType::CProc:
            {
                std::vector<Value> args;
                args.reserve(argc);
                for (uint16_t j=0; j < argc; ++j)
                    args.push_back(pop());
                // reverse arguments
                std::reverse(args.begin(), args.end());
                // call proc
                Value return_value = function.proc()(args);
                push(return_value);
                return;
            }

            // is it a user defined function?
            case ValueType::Closure:
            {
                int p = m_frames.size() - 1;

                Closure c = function.closure();
                // load saved frame
                m_frames.push_back(c.frame());
                // create dedicated frame
                m_frames.push_back(std::make_shared<Frame>(m_symbols.size(), m_ip, m_pp));
                m_pp = c.pageAddr();
                m_ip = -1;  // because we are doing a m_ip++ right after that
                for (std::size_t j=0; j < argc; ++j)
                    push(pop(p));
                return;
            }

            default:
                throwVMError("couldn't identify function object");
        }
    }

    inline void VM::saveEnv()
    {
        /*
            Argument: none
            Job: Used to tell the Virtual Machine to save the current environment. Main goal is
                    to be able to handle closures, which need to save the environment in which
                    they were created
        */
        m_saved_frame = m_frames.size() - 1;
    }

    inline void VM::builtin()
    {
        /*
            Argument: id of builtin (two bytes, big endian)
            Job: Push the builtin function object on the stack
        */
        ++m_ip;
        auto id = readNumber();

        if (m_debug)
            Ark::logger.info("BUILTIN ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

        push(FFI::builtins[id].second);
    }

    inline void VM::mut()
    {
        /*
            Argument: symbol id (two bytes, big endian)
            Job: Take the value on top of the stack and create a variable in the current scope,
                named following the given symbol id (cf symbols table)
        */
        ++m_ip;
        auto id = readNumber();

        if (m_debug)
            Ark::logger.info("MUT ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

        backFrame()[id] = pop();
        if (backFrame()[id].valueType() == ValueType::Closure)
            backFrame()[id].closure_ref().save(m_frames.size() - 1, id);
    }

    inline void VM::del()
    {
        /*
            Argument: symbol id (two bytes, big endian)
            Job: Remove a variable/constant named following the given symbol id (cf symbols table)
        */
        ++m_ip;
        auto id = readNumber();

        if (m_debug)
            Ark::logger.info("DEL ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

        for (auto it=m_frames.rbegin(); it != m_frames.rend(); ++it)
        {
            if ((*it)->find(id))
            {
                // delete it
                (**it)[id] = FFI::nil;
                return;
            }
        }

        throwVMError("couldn't find symbol: " + m_symbols[id]);
    }

    inline void VM::operators(uint8_t inst)
    {
        /*
            Handling the operator instructions
        */
        switch (inst)
        {
            case Instruction::ADD:
            {
                auto b = pop(), a = pop();
                if (a.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of + should be Numbers");
                if (b.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of + should be Numbers");
                
                push(Value(a.number() + b.number()));
                break;
            }

            case Instruction::SUB:
            {
                auto b = pop(), a = pop();
                if (a.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of - should be Numbers");
                if (b.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of - should be Numbers");
                
                push(Value(a.number() - b.number()));
                break;
            }

            case Instruction::MUL:
            {
                auto b = pop(), a = pop();
                if (a.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of * should be Numbers");
                if (b.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of * should be Numbers");
                
                push(Value(a.number() * b.number()));
                break;
            }

            case Instruction::DIV:
            {
                auto b = pop(), a = pop();
                if (a.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of / should be Numbers");
                if (b.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of / should be Numbers");
                
                auto d = b.number();
                if (d == 0)
                    throw Ark::ZeroDivisionError();
                
                push(Value(a.number() / d));
                break;
            }

            case Instruction::GT:
            {
                auto b = pop(), a = pop();
                if (a.valueType() == ValueType::String)
                {
                    if (b.valueType() != ValueType::String)
                        throw Ark::TypeError("Arguments of > should have the same type");
                    
                    push((a.string() > b.string()) ? FFI::trueSym : FFI::falseSym);
                    break;
                }
                else if (a.valueType() == ValueType::Number)
                {
                    if (b.valueType() != ValueType::Number)
                        throw Ark::TypeError("Arguments of > should have the same type");
                    
                    push((a.number() > b.number()) ? FFI::trueSym : FFI::falseSym);
                    break;
                }
                throw Ark::TypeError("Arguments of > should either be Strings or Numbers");
            }
            
            case Instruction::LT:
            {
                auto b = pop(), a = pop();
                if (a.valueType() == ValueType::String)
                {
                    if (b.valueType() != ValueType::String)
                        throw Ark::TypeError("Arguments of < should have the same type");
                    
                    push((a.string() < b.string()) ? FFI::trueSym : FFI::falseSym);
                    break;
                }
                else if (a.valueType() == ValueType::Number)
                {
                    if (b.valueType() != ValueType::Number)
                        throw Ark::TypeError("Arguments of < should have the same type");
                    
                    push((a.number() < b.number()) ? FFI::trueSym : FFI::falseSym);
                    break;
                }
                throw Ark::TypeError("Arguments of < should either be Strings or Numbers");
            }

            case Instruction::LE:
            {
                auto b = pop(), a = pop();
                if (a.valueType() == ValueType::String)
                {
                    if (b.valueType() != ValueType::String)
                        throw Ark::TypeError("Arguments of <= should have the same type");
                    
                    push((a.string() <= b.string()) ? FFI::trueSym : FFI::falseSym);
                    break;
                }
                else if (a.valueType() == ValueType::Number)
                {
                    if (b.valueType() != ValueType::Number)
                        throw Ark::TypeError("Arguments of <= should have the same type");
                    
                    push((a.number() <= b.number()) ? FFI::trueSym : FFI::falseSym);
                    break;
                }
                throw Ark::TypeError("Arguments of <= should either be Strings or Numbers");
            }

            case Instruction::GE:
            {
                auto b = pop(), a = pop();
                if (a.valueType() == ValueType::String)
                {
                    if (b.valueType() != ValueType::String)
                        throw Ark::TypeError("Arguments of >= should have the same type");
                    
                    push((a.string() >= b.string()) ? FFI::trueSym : FFI::falseSym);
                    break;
                }
                else if (a.valueType() == ValueType::Number)
                {
                    if (b.valueType() != ValueType::Number)
                        throw Ark::TypeError("Arguments of >= should have the same type");
                    
                    push((a.number() >= b.number()) ? FFI::trueSym : FFI::falseSym);
                    break;
                }
                throw Ark::TypeError("Arguments of >= should either be Strings or Numbers");
            }

            case Instruction::NEQ:
            {
                push(!(pop() == pop()) ? FFI::trueSym : FFI::falseSym);
                break;
            }

            case Instruction::EQ:
            {
                push((pop() == pop()) ? FFI::trueSym : FFI::falseSym);
                break;
            }

            case Instruction::LEN:
            {
                auto a = pop();
                if (a.valueType() == ValueType::List)
                {
                    push(Value(static_cast<int>(a.const_list().size())));
                    break;
                }
                if (a.valueType() == ValueType::String)
                {
                    push(Value(static_cast<int>(a.string().size())));
                    break;
                }

                throw Ark::TypeError("Argument of len must be a list or a String");
            }

            case Instruction::EMPTY:
            {
                auto a = pop();
                if (a.valueType() != ValueType::List)
                    throw Ark::TypeError("Argument of empty must be a list");
                
                push((a.const_list().size() == 0) ? FFI::trueSym : FFI::falseSym);
                break;
            }

            case Instruction::FIRSTOF:
            {
                auto a = pop();
                if (a.valueType() != ValueType::List)
                    throw Ark::TypeError("Argument of firstof must be a list");
                
                push(a.const_list()[0]);
                break;
            }

            case Instruction::TAILOF:
            {
                auto a = pop();
                if (a.valueType() != ValueType::List)
                    throw Ark::TypeError("Argument of tailof must be a list");
                
                if (a.const_list().size() < 2)
                {
                    push(FFI::nil);
                    break;
                }
                
                a.list().erase(a.const_list().begin());
                push(a);
                break;
            }

            case Instruction::HEADOF:
            {
                auto a = pop();
                if (a.valueType() != ValueType::List)
                    throw Ark::TypeError("Argument of headof must be a list");
                
                if (a.const_list().size() < 2)
                {
                    push(FFI::nil);
                    break;
                }
                
                a.list().erase(a.const_list().end());
                push(a);
                break;
            }

            case Instruction::ISNIL:
            {
                push((pop() == FFI::nil) ? FFI::trueSym : FFI::falseSym);
                break;
            }

            case Instruction::ASSERT:
            {
                auto b = pop(), a = pop();
                if (a == FFI::falseSym)
                {
                    if (b.valueType() != ValueType::String)
                        throw Ark::TypeError("Second argument of assert must be a String");

                    throw Ark::AssertionFailed(b.string());
                }
                push(FFI::nil);
                break;
            }

            case Instruction::TO_NUM:
            {
                auto a = pop();
                if (a.valueType() != ValueType::String)
                    throw Ark::TypeError("Argument of toNumber must be a String");
                
                push(Value(std::stod(a.string().c_str())));
                break;
            }

            case Instruction::TO_STR:
            {
                std::stringstream ss;
                ss << pop();
                push(Value(ss.str()));
                break;
            }

            case Instruction::AT:
            {
                auto b = pop(), a = pop();
                if (a.valueType() != ValueType::List)
                    throw Ark::TypeError("Argument 1 of @ should be a List");
                if (b.valueType() != ValueType::Number)
                    throw Ark::TypeError("Argument 2 of @ should be a Number");
                
                push(a.const_list()[static_cast<long>(b.number())]);
                break;
            }

            case Instruction::AND_:
            {
                push(pop() == FFI::trueSym && pop() == FFI::trueSym);
                break;
            }

            case Instruction::OR_:
            {
                push(pop() == FFI::trueSym || pop() == FFI::trueSym);
                break;
            }

            case Instruction::MOD:
            {
                auto b = pop(), a = pop();
                if (a.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of mod should be Numbers");
                if (b.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of mod should be Numbers");
                
                push(Value(std::fmod(a.number(), b.number())));
                break;
            }
        }
    }
}