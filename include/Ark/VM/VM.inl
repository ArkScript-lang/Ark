template<bool debug>
VM_t<debug>::VM_t(bool persist) :
    m_persist(persist), m_ip(0), m_pp(0), m_running(false), m_filename("FILE"), m_last_sym_loaded({0, nullptr})
{}

// ------------------------------------------
//            bytecode loading
// ------------------------------------------

template<bool debug>
void VM_t<debug>::feed(const std::string& filename)
{
    try
    {
        Ark::BytecodeReader bcr;
        bcr.feed(filename);
        m_bytecode = bcr.bytecode();

        m_filename = filename;

        configure();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

template<bool debug>
void VM_t<debug>::feed(const bytecode_t& bytecode)
{
    m_bytecode = bytecode;
    configure();
}

template<bool debug>
void VM_t<debug>::configure()
{
    using namespace Ark::internal;

    // configure tables and pages
    const bytecode_t& b = m_bytecode;
    std::size_t i = 0;

    auto readNumber = [&b] (std::size_t& i) -> uint16_t {
        uint16_t x = (static_cast<uint16_t>(b[i]) << 8); ++i;
        uint16_t y = static_cast<uint16_t>(b[i]);
        return x + y;
    };

    // read tables and check if bytecode is valid
    if (!(b.size() > 4 && b[i++] == 'a' && b[i++] == 'r' && b[i++] == 'k' && b[i++] == Instruction::NOP))
        throwVMError("invalid format: couldn't find magic constant");

    if constexpr (debug)
        Ark::logger.info("(Virtual Machine) magic constant found: ark\\0");

    uint16_t major = readNumber(i); i++;
    uint16_t minor = readNumber(i); i++;
    uint16_t patch = readNumber(i); i++;

    if constexpr (debug)
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
    timestamp_t timestamp = 0;
    auto aa = (static_cast<timestamp_t>(m_bytecode[  i]) << 56),
            ba = (static_cast<timestamp_t>(m_bytecode[++i]) << 48),
            ca = (static_cast<timestamp_t>(m_bytecode[++i]) << 40),
            da = (static_cast<timestamp_t>(m_bytecode[++i]) << 32),
            ea = (static_cast<timestamp_t>(m_bytecode[++i]) << 24),
            fa = (static_cast<timestamp_t>(m_bytecode[++i]) << 16),
            ga = (static_cast<timestamp_t>(m_bytecode[++i]) <<  8),
            ha = (static_cast<timestamp_t>(m_bytecode[++i]));
    i++;
    timestamp = aa + ba + ca + da + ea + fa + ga + ha;

    if constexpr (debug)
        Ark::logger.info("(Virtual Machine) timestamp: ", timestamp);

    if (b[i] == Instruction::SYM_TABLE_START)
    {
        if constexpr (debug)
            Ark::logger.info("(Virtual Machine) symbols table");
        
        i++;
        uint16_t size = readNumber(i);
        m_symbols.reserve(size);
        i++;

        if constexpr (debug)
            Ark::logger.info("(Virtual Machine) length:", size);
        
        for (uint16_t j=0; j < size; ++j)
        {
            std::string symbol = "";
            while (b[i] != 0)
                symbol.push_back(b[i++]);
            i++;

            m_symbols.push_back(symbol);

            if constexpr (debug)
                Ark::logger.info("(Virtual Machine) -", symbol);
        }
    }
    else
        throwVMError("couldn't find symbols table");

    if (b[i] == Instruction::VAL_TABLE_START)
    {
        if constexpr (debug)
            Ark::logger.info("(Virtual Machine) constants table");
        
        i++;
        uint16_t size = readNumber(i);
        m_constants.reserve(size);
        i++;

        if constexpr (debug)
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
                
                if constexpr (debug)
                    Ark::logger.info("(Virtual Machine) - (Number)", val);
            }
            else if (type == Instruction::STRING_TYPE)
            {
                std::string val = "";
                while (b[i] != 0)
                    val.push_back(b[i++]);
                i++;

                m_constants.emplace_back(val);
                
                if constexpr (debug)
                    Ark::logger.info("(Virtual Machine) - (String)", val);
            }
            else if (type == Instruction::FUNC_TYPE)
            {
                uint16_t addr = readNumber(i);
                i++;

                m_constants.emplace_back(addr);

                if constexpr (debug)
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
        if constexpr (debug)
            Ark::logger.info("(Virtual Machine) plugins table");
        
        i++;
        uint16_t size = readNumber(i);
        m_plugins.reserve(size);
        i++;

        if constexpr (debug)
            Ark::logger.info("(Virtual Machine) length:", size);
        
        for (uint16_t j=0; j < size; ++j)
        {
            std::string plugin = "";
            while (b[i] != 0)
                plugin.push_back(b[i++]);
            i++;

            m_plugins.push_back(plugin);

            if constexpr (debug)
                Ark::logger.info("(Virtual Machine) -", plugin);
        }
    }
    else
        throwVMError("couldn't find plugins table");
    
    while (b[i] == Instruction::CODE_SEGMENT_START)
    {
        if constexpr (debug)
            Ark::logger.info("(Virtual Machine) code segment");
        
        i++;
        uint16_t size = readNumber(i);
        i++;

        if constexpr (debug)
            Ark::logger.info("(Virtual Machine) length:", size);
        
        m_pages.emplace_back();
        m_pages.back().reserve(size);

        for (uint16_t j=0; j < size; ++j)
            m_pages.back().push_back(b[i++]);
        
        if (i == b.size())
            break;
    }
}

template<bool debug>
void VM_t<debug>::loadFunction(const std::string& name, internal::Value::ProcType function)
{
    using namespace Ark::internal;

    // put it in the global frame, aka the first one
    auto it = std::find(m_symbols.begin(), m_symbols.end(), name);
    if (it == m_symbols.end())
    {
        if constexpr (debug)
            Ark::logger.warn("Couldn't find symbol with name", name, "to set its value as a function");
        return;
    }

    registerVariable(std::distance(m_symbols.begin(), it), Value(function));
}

// ------------------------------------------
//                 execution
// ------------------------------------------

template<bool debug>
void VM_t<debug>::run()
{
    using namespace Ark::internal;

    // reset VM before each run
    m_ip = 0;
    m_pp = 0;

    if (!m_persist)
    {
        m_frames.clear();
        m_frames.reserve(100);
        m_frames.emplace_back();

        m_saved_frame.reset();

        m_locals.clear();
        m_locals.reserve(32);

        // loading plugins
        for (const auto& file: m_plugins)
        {
            namespace fs = std::filesystem;

            std::string path = "./" + file;
            if (m_filename != "FILE")  // bytecode loaded from file
                path = "./" + (fs::path(m_filename).parent_path() / fs::path(file)).string();
            std::string lib_path = (fs::path(ARK_STD) / fs::path(file)).string();

            if constexpr (debug)
                Ark::logger.info("Loading", file, "in", path, "or in", lib_path);

            if (Ark::Utils::fileExists(path))  // if it exists alongside the .arkc file
                m_shared_lib_objects.emplace_back(path);
            else if (Ark::Utils::fileExists(lib_path))  // check in LOAD_PATH otherwise
                m_shared_lib_objects.emplace_back(lib_path);
            else
                throwVMError("could not load plugin " + file);

            // load data from it!
            using Mapping_t = std::unordered_map<std::string, Value::ProcType>;
            using map_fun_t = Mapping_t (*) ();
            Mapping_t map = m_shared_lib_objects.back().template get<map_fun_t>("getFunctionsMapping")();

            for (auto&& kv : map)
            {
                // put it in the global frame, aka the first one
                auto it = std::find(m_symbols.begin(), m_symbols.end(), kv.first);
                if (it != m_symbols.end())
                {
                    if constexpr (debug)
                        Ark::logger.info("Loading", kv.first);

                    registerVariable(std::distance(m_symbols.begin(), it), Value(kv.second));
                }
            }
        }
    }

    if constexpr (debug)
        Ark::logger.info("Starting at PP:{0}, IP:{1}"s, m_pp, m_ip);

    //try {
        m_running = true;
        while (m_running)
        {
            if constexpr (debug)
            {
                if (m_pp >= m_pages.size())
                    throwVMError("page pointer has gone too far (" + Ark::Utils::toString(m_pp) + ")");
                if (m_ip >= m_pages[m_pp].size())
                    throwVMError("instruction pointer has gone too far (" + Ark::Utils::toString(m_ip) + ")");
            }

            // get current instruction
            uint8_t inst = m_pages[m_pp][m_ip];

            // and it's time to du-du-du-du-duel!
            if (Instruction::FIRST_COMMAND <= inst && inst <= Instruction::LAST_COMMAND)
                switch (inst)
                {
                case Instruction::NOP:
                    // do nothing
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
            else
                throwVMError("unknown instruction: " + Ark::Utils::toString(static_cast<std::size_t>(inst)) +
                    ", pp: " + Ark::Utils::toString(m_pp) + ", ip: " + Ark::Utils::toString(m_ip)
                );
            
            // move forward
            ++m_ip;
        }
    /*} catch (const std::exception& e) {
        std::cout << "At IP: " << m_ip << ", PP: " << m_pp << "\n";
        std::cout << "Locals:\n";
        int count = 0;
        for (auto&& p: m_locals)
        {
            std::cout << p.first << " -> " << p.second << "\n";

            count++;

            // if (count > 10) { std::cout << "...\n"; break; }
        }
        std::cout << e.what() << std::endl;
    }*/
}

// ------------------------------------------
//            stack management
// ------------------------------------------

template<bool debug>
inline internal::Value&& VM_t<debug>::pop(int page)
{
    if (page == -1)
        return m_frames.back().pop();
    return m_frames[static_cast<std::size_t>(page)].pop();
}

template<bool debug>
void VM_t<debug>::push(const internal::Value& value)
{
    m_frames.back().push(value);
}

template<bool debug>
inline void VM_t<debug>::push(internal::Value&& value)
{
    m_frames.back().push(value);
}

// ------------------------------------------
//               instructions
// ------------------------------------------

template<bool debug>
inline void VM_t<debug>::loadSymbol()
{
    /*
        Argument: symbol id (two bytes, big endian)
        Job: Load a symbol from its id onto the stack
    */
    ++m_ip;
    auto id = readNumber();

    if constexpr (debug)
        Ark::logger.info("LOAD_SYMBOL ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

    if (auto var = findNearestVariable(id))
    {
        push(*var.value());
        m_last_sym_loaded.first = id;
        m_last_sym_loaded.second = var.value();
        return;
    }

    throwVMError("couldn't find symbol to load: " + m_symbols[id]);
}

template<bool debug>
inline void VM_t<debug>::loadConst()
{
    /*
        Argument: constant id (two bytes, big endian)
        Job: Load a constant from its id onto the stack. Should check for a saved environment
                and push a Closure with the page address + environment instead of the constant
    */
    using namespace Ark::internal;

    ++m_ip;
    auto id = readNumber();

    if constexpr (debug)
        Ark::logger.info("LOAD_CONST ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);
    
    if (m_saved_frame && m_constants[id].valueType() == ValueType::PageAddr)
    {
        push(Value(
            Closure(
                m_constants[id].pageAddr(),
                m_locals.begin() + m_frames.back().localsStart(),
                m_locals.end()
            )
        ));
        m_saved_frame.reset();
    }
    else
        push(m_constants[id]);
}

template<bool debug>
inline void VM_t<debug>::popJumpIfTrue()
{
    /*
        Argument: absolute address to jump to (two bytes, big endian)
        Job: Jump to the provided address if the last value on the stack was equal to true.
                Remove the value from the stack no matter what it is
    */
    using namespace Ark::internal;

    ++m_ip;
    int16_t addr = static_cast<int16_t>(readNumber());

    if constexpr (debug)
        Ark::logger.info("POP_JUMP_IF_TRUE ({0}) PP:{1}, IP:{2}"s, addr, m_pp, m_ip);

    if (pop() == FFI::trueSym)
        m_ip = addr - 1;  // because we are doing a ++m_ip right after this
}

template<bool debug>
inline void VM_t<debug>::store()
{
    /*
        Argument: symbol id (two bytes, big endian)
        Job: Take the value on top of the stack and put it inside a variable named following
                the symbol id (cf symbols table), in the nearest scope. Raise an error if it
                couldn't find a scope where the variable exists
    */
    using namespace Ark::internal;
    
    ++m_ip;
    auto id = readNumber();

    if constexpr (debug)
        Ark::logger.info("STORE ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

    if (auto var = findNearestVariable(id))
    {
        *var.value() = pop();
        return;
    }

    throwVMError("couldn't find symbol: " + m_symbols[id]);
}

template<bool debug>
inline void VM_t<debug>::let()
{
    /*
        Argument: symbol id (two bytes, big endian)
        Job: Take the value on top of the stack and create a constant in the current scope, named
                following the given symbol id (cf symbols table)
    */
    using namespace Ark::internal;

    ++m_ip;
    auto id = readNumber();

    if constexpr (debug)
        Ark::logger.info("LET ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);
    
    // check if we are redefining a variable
    if (findInCurrentScope(id))
        throwVMError("can not use 'let' to redefine a symbol");

    registerVariable(id, pop()).setConst(true);
}

template<bool debug>
inline void VM_t<debug>::popJumpIfFalse()
{
    /*
        Argument: absolute address to jump to (two bytes, big endian)
        Job: Jump to the provided address if the last value on the stack was equal to false. Remove
                the value from the stack no matter what it is
    */
    using namespace Ark::internal;

    ++m_ip;
    int16_t addr = static_cast<int16_t>(readNumber());

    if constexpr (debug)
        Ark::logger.info("POP_JUMP_IF_FALSE ({0}) PP:{1}, IP:{2}"s, addr, m_pp, m_ip);

    if (pop() == FFI::falseSym)
        m_ip = addr - 1;  // because we are doing a ++m_ip right after this
}

template<bool debug>
inline void VM_t<debug>::jump()
{
    /*
        Argument: absolute address to jump to (two byte, big endian)
        Job: Jump to the provided address
    */
    using namespace Ark::internal;

    ++m_ip;
    int16_t addr = static_cast<int16_t>(readNumber());

    if constexpr (debug)
        Ark::logger.info("JUMP ({0}) PP:{1}, IP:{2}"s, addr, m_pp, m_ip);

    m_ip = addr - 1;  // because we are doing a ++m_ip right after this
}

template<bool debug>
inline void VM_t<debug>::ret()
{
    /*
        Argument: none
        Job: If in a code segment other than the main one, quit it, and push the value on top of
                the stack to the new stack ; should as well delete the current environment.
                Otherwise, acts as a `HALT`
    */
    using namespace Ark::internal;

    if constexpr (debug)
        Ark::logger.info("RET PP:{0}, IP:{1}"s, m_pp, m_ip);
    
    // check if we should halt the VM
    if (m_pp == 0)
    {
        m_running = false;
        return;
    }

    // save pp
    PageAddr_t old_pp = static_cast<PageAddr_t>(m_pp);
    m_pp = m_frames.back().callerPageAddr();
    m_ip = m_frames.back().callerAddr();

    auto rm_frame = [this] () -> void {
        auto locals_start = m_frames.back().localsStart();
        // save env if needed
        if (auto id = m_frames.back().getClosure())
        {
            if (id.value().second->valueType() == ValueType::Closure)
            {
                if constexpr (debug)
                    Ark::logger.warn("Saving data back into the closure:", m_symbols[id.value().first]);
                Value* closure = id.value().second;
                if constexpr (debug)
                {
                    for (auto it=m_locals.begin() + locals_start; it != m_locals.end(); ++it)
                        Ark::logger.data(m_symbols[it->first].c_str(), it->first, "=>", it->second);
                }
                closure->closure_ref().save(m_locals.begin() + locals_start, m_locals.end());
                if constexpr (debug)
                {
                    for (auto&& id_val: closure->closure_ref().bindedVars())
                        Ark::logger.data(m_symbols[id_val.first].c_str(), id_val.first, "=>", id_val.second);
                }
            }
        }
        // remove frame
        m_frames.pop_back();
        // clear locals
        m_locals.erase(m_locals.begin() + locals_start, m_locals.end());
    };
    
    if (m_frames.back().stackSize() != 0)
    {
        Value return_value(pop());
        rm_frame();
        // push value as the return value of a function to the current stack
        push(return_value);
    }
    else
        rm_frame();
}

template<bool debug>
inline void VM_t<debug>::call()
{
    /*
        Argument: number of arguments when calling the function
        Job: Call function from its symbol id located on top of the stack. Take the given number of
                arguments from the top of stack and give them  to the function (the first argument taken
                from the stack will be the last one of the function). The stack of the function is now composed
                of its arguments, from the first to the last one
    */
    using namespace Ark::internal;

    ++m_ip;
    auto argc = readNumber();

    if constexpr (debug)
        Ark::logger.info("CALL ({0}) PP:{1}, IP:{2}"s, argc, m_pp, m_ip);

    Value function(pop());

    switch (function.valueType())
    {
        // is it a builtin function name?
        case ValueType::CProc:
        {
            // drop arguments from the stack
            std::vector<Value> args(argc);
            for (uint16_t j=0; j < argc; ++j)
                args[j] = pop();
            
            // reverse arguments
            std::reverse(args.begin(), args.end());
            // call proc
            push(function.proc()(args));
            return;
        }

        // is it a user defined function?
        case ValueType::PageAddr:
        {
            int old_frame = m_frames.size() - 1;
            auto new_page_pointer = function.pageAddr();

            // create dedicated frame
            m_frames.emplace_back(m_ip, m_pp, m_locals.size());
            // store "reference" to the function
            if (!findInCurrentScope(m_last_sym_loaded.first))
                registerVariable(m_last_sym_loaded.first, std::move(function));

            m_pp = new_page_pointer;
            m_ip = -1;  // because we are doing a m_ip++ right after that
            for (std::size_t j=0; j < argc; ++j)
                push(pop(old_frame));
            return;
        }

        // is it a user defined closure?
        case ValueType::Closure:
        {
            int old_frame = m_frames.size() - 1;
            Closure c = function.closure();
            auto new_page_pointer = c.pageAddr();

            // create dedicated frame
            m_frames.emplace_back(m_ip, m_pp, m_locals.size());
            if constexpr (debug)
                Ark::logger.data("Saving closure: {0} ({1})"s, m_symbols[m_last_sym_loaded.first], m_last_sym_loaded.first);
            m_frames.back().setClosure(m_last_sym_loaded);
            // store "reference" to the function
            if (!findInCurrentScope(m_last_sym_loaded.first))
                registerVariable(m_last_sym_loaded.first, std::move(function));
            // copy variables captured by the closure to the "execution scope"
            for (auto&& id_val: c.bindedVars())
            {
                if constexpr (debug)
                    Ark::logger.data(m_symbols[id_val.first].c_str(), "=>", id_val.second);
                registerVariable(id_val.first, id_val.second);
            }

            m_pp = new_page_pointer;
            m_ip = -1;  // because we are doing a m_ip++ right after that
            for (std::size_t j=0; j < argc; ++j)
                push(pop(old_frame));
            return;
        }

        default:
            throwVMError("couldn't identify function object: type index " + Ark::Utils::toString(static_cast<int>(function.valueType())));
    }
}

template<bool debug>
inline void VM_t<debug>::saveEnv()
{
    /*
        Argument: none
        Job: Used to tell the Virtual Machine to save the current environment. Main goal is
                to be able to handle closures, which need to save the environment in which
                they were created
    */
    m_saved_frame = m_frames.size() - 1;
}

template<bool debug>
inline void VM_t<debug>::builtin()
{
    /*
        Argument: id of builtin (two bytes, big endian)
        Job: Push the builtin function object on the stack
    */
    using namespace Ark::internal;

    ++m_ip;
    auto id = readNumber();

    if constexpr (debug)
        Ark::logger.info("BUILTIN ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

    push(FFI::builtins[id].second);
}

template<bool debug>
inline void VM_t<debug>::mut()
{
    /*
        Argument: symbol id (two bytes, big endian)
        Job: Take the value on top of the stack and create a variable in the current scope,
            named following the given symbol id (cf symbols table)
    */
    using namespace Ark::internal;

    ++m_ip;
    auto id = readNumber();

    if constexpr (debug)
        Ark::logger.info("MUT ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

    registerVariable(id, pop());
}

template<bool debug>
inline void VM_t<debug>::del()
{
    /*
        Argument: symbol id (two bytes, big endian)
        Job: Remove a variable/constant named following the given symbol id (cf symbols table)
    */
    using namespace Ark::internal;

    ++m_ip;
    auto id = readNumber();

    if constexpr (debug)
        Ark::logger.info("DEL ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);
    
    if (auto var = findNearestVariable(id))
    {
        *var.value() = FFI::nil;
        return;
    }

    throwVMError("couldn't find symbol: " + m_symbols[id]);
}

template<bool debug>
inline void VM_t<debug>::operators(uint8_t inst)
{
    /*
        Handling the operator instructions
    */
    using namespace Ark::internal;
    
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
            auto a = pop();
            push(pop() == FFI::trueSym || a == FFI::trueSym);
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