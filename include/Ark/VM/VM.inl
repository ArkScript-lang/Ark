template<bool debug>
VM_t<debug>::VM_t(bool persist) :
    m_persist(persist), m_libdir(ARK_STD_DEFAULT), m_ip(0), m_pp(0), m_running(false), m_filename("FILE"),
    m_last_sym_loaded(0), m_until_frame_count(0)
{}

template<bool debug>
VM_t<debug>::VM_t(const std::string& lib_dir) :
    m_persist(false), m_libdir(lib_dir != "" ? lib_dir : ARK_STD_DEFAULT), m_ip(0), m_pp(0), m_running(false), m_filename("FILE"),
    m_last_sym_loaded(0), m_until_frame_count(0)
{}

// ------------------------------------------
//            bytecode loading
// ------------------------------------------

template<bool debug>
bool VM_t<debug>::feed(const std::string& filename)
{
    bool result = true;
    try {
        Ark::BytecodeReader bcr;
        bcr.feed(filename);
        m_bytecode = bcr.bytecode();

        m_filename = filename;

        configure();
        init();
    } catch (const std::exception& e) {
        result = false;
        std::cout << e.what() << std::endl;
    }

    return result;
}

template<bool debug>
bool VM_t<debug>::feed(const bytecode_t& bytecode)
{
    bool result = true;
    try {
        m_bytecode = bytecode;
        configure();
        init();
    } catch (const std::exception& e) {
        result = false;
        std::cout << e.what() << std::endl;
    }

    return result;
}

static bool compile(bool debug, const std::string& file, const std::string& output, const std::string& lib_dir)
{
    Compiler compiler(debug, lib_dir);

    try {
        compiler.feed(Utils::readFile(file), file);
        compiler.compile();

        if (output != "")
            compiler.saveTo(output);
        else
            compiler.saveTo(file.substr(0, file.find_last_of('.')) + ".arkc");
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown lexer-or-parser-compiler error" << std::endl;
        return false;
    }

    return true;
}

template<bool debug>
void VM_t<debug>::doFile(const std::string& file)
{
    if (!Ark::Utils::fileExists(file))
    {
        Ark::logger.error("Can not find file '" + file + "'");
        return;
    }

    // check if it's a bytecode file or a source code file
    Ark::BytecodeReader bcr;
    try {
        bcr.feed(file);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return;
    }

    if (bcr.timestamp() == 0)  // couldn't read magic number, it's a source file
    {
        // check if it's in the arkscript cache
        std::string short_filename = Ark::Utils::getFilenameFromPath(file);
        std::string filename = short_filename.substr(0, short_filename.find_last_of('.')) + ".arkc";
        std::filesystem::path directory =  (std::filesystem::path(file)).parent_path() / ARK_CACHE_DIRNAME;
        std::string path = (directory / filename).string();

        bool compiled_successfuly = false;

        if (Ark::Utils::fileExists(path))
        {
            auto ftime = std::filesystem::last_write_time(std::filesystem::path(file));

            // this shouldn't fail
            Ark::BytecodeReader bcr2;
            bcr2.feed(path);
            auto timestamp = bcr2.timestamp();
            auto file_last_write = static_cast<decltype(timestamp)>(std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count());
            // recompile
            if (timestamp < file_last_write)
                compiled_successfuly = Ark::compile(debug, file, path, m_libdir);
            else
                compiled_successfuly = true;
        }
        else
        {
            if (!std::filesystem::exists(directory))  // create ark cache directory
                std::filesystem::create_directory(directory);
            
            compiled_successfuly = Ark::compile(debug, file, path, m_libdir);
        }
        
        // run
        if (compiled_successfuly)
        {
            if (feed(path))
                run();
        }
    }
    else  // it's a bytecode file, run it
    {
        if (feed(file))
            run();
    }
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
void VM_t<debug>::init()
{
    using namespace Ark::internal;

    // clearing frames and setting up a new one
    m_frames.clear();
    m_frames.emplace_back();

    m_saved_scope.reset();

    // clearing locals (scopes) and create a global scope
    m_locals.clear();
    createNewScope();

    // loading binded functions
    // put them in the global frame if we can, aka the first one
    for (auto name_func : m_binded_functions)
    {
        auto it = std::find(m_symbols.begin(), m_symbols.end(), name_func.first);
        if (it == m_symbols.end())
        {
            if constexpr (debug)
                Ark::logger.warn("Couldn't find symbol with name", name_func.first, "to set its value as a function");
        }
        else
            registerVariable<0>(std::distance(m_symbols.begin(), it), Value(name_func.second));
    }

    // loading plugins
    // we don't want to load the plugins multiple times
    if (m_shared_lib_objects.size() == m_plugins.size())
        return;
    
    for (const auto& file : m_plugins)
    {
        namespace fs = std::filesystem;

        std::string path = "./" + file;
        if (m_filename != "FILE")  // bytecode loaded from file
            path = "./" + (fs::path(m_filename).parent_path() / fs::path(file)).string();
        std::string lib_path = (fs::path(m_libdir) / fs::path(file)).string();

        if constexpr (debug)
            Ark::logger.info("Loading", file, "in", path, "or in", lib_path);

        if (Ark::Utils::fileExists(path))  // if it exists alongside the .arkc file
            m_shared_lib_objects.emplace_back(path);
        else if (Ark::Utils::fileExists(lib_path))  // check in LOAD_PATH otherwise
            m_shared_lib_objects.emplace_back(lib_path);
        else
            throwVMError("could not load plugin " + file);
        
        if constexpr (debug)
            Ark::logger.info("Plugin loaded");

        // load data from it!
        using Mapping_t = std::unordered_map<std::string, Value::ProcType>;
        using map_fun_t = Mapping_t(*) ();
        Mapping_t map = m_shared_lib_objects.back().template get<map_fun_t>("getFunctionsMapping")();

        if constexpr (debug)
            Ark::logger.info("Functions mapping retrieved\n{0} symbols"s, map.size());

        for (auto&& kv : map)
        {
            // put it in the global frame, aka the first one
            auto it = std::find(m_symbols.begin(), m_symbols.end(), kv.first);
            if (it != m_symbols.end())
            {
                if constexpr (debug)
                    Ark::logger.info("Loading", kv.first);

                registerVariable<0>(std::distance(m_symbols.begin(), it), Value(kv.second));
            }
        }
    }
}

template<bool debug>
void VM_t<debug>::loadFunction(const std::string& name, internal::Value::ProcType function)
{
    m_binded_functions[name] = std::move(function);
}

template<bool debug>
internal::Value& VM_t<debug>::operator[](const std::string& name)
{
    using namespace Ark::internal;

    // find id of object
    auto it = std::find(m_symbols.begin(), m_symbols.end(), name);
    if (it == m_symbols.end())
        return FFI::nil;

    uint16_t id = static_cast<uint16_t>(std::distance(m_symbols.begin(), it));
    auto var = findNearestVariable(id);
    if (var != nullptr)
        return *var;
    else
        return FFI::nil;
}

// ------------------------------------------
//                 execution
// ------------------------------------------

template<bool debug>
void VM_t<debug>::run()
{
    using namespace Ark::internal;

    if constexpr (debug)
        Ark::logger.info("Starting at PP:{0}, IP:{1}"s, m_pp, m_ip);

    safeRun();

    // reset VM after each run
    m_ip = 0;
    m_pp = 0;

    if (!m_persist)
        init();
}

template<bool debug>
void VM_t<debug>::safeRun(std::size_t untilFrameCount)
{
    using namespace Ark::internal;
    m_until_frame_count = untilFrameCount;
    
    try {
        m_running = true;
        while (m_running && m_frames.size() > m_until_frame_count)
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
            if (inst == Instruction::NOP)
            {
                if constexpr (debug)
                    Ark::logger.info("NOP PP:{0}, IP:{1}"s, m_pp, m_ip);
            }
            else if (Instruction::FIRST_COMMAND <= inst && inst <= Instruction::LAST_COMMAND)
                switch (inst)
                {
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
                
                case Instruction::CAPTURE:
                    capture();
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
                
                case Instruction::SAVE_ENV:
                    saveEnv();
                    break;
                
                case Instruction::GET_FIELD:
                    getField();
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
    } catch (const std::exception& e) {
        std::cerr << "\n" << termcolor::red << e.what() << "\n";
        std::cerr << termcolor::reset << "At IP: " << m_ip << ", PP: " << m_pp << "\n";

        if (m_frames.size() > 1)
        {
            // display call stack trace
            for (auto it=m_frames.rbegin(); it != m_frames.rend(); ++it)
            {
                std::cerr << "[" << termcolor::cyan << std::distance(it, m_frames.rend()) << termcolor::reset << "] ";
                if (it->currentPageAddr() != 0)
                {
                    uint16_t id = findNearestVariableIdWithValue(
                        Value(static_cast<PageAddr_t>(it->currentPageAddr()))
                    );
                    
                    std::cerr << "In function `" << termcolor::green << m_symbols[id] << termcolor::reset << "'\n";
                }
                else
                    std::cerr << "In global scope\n";

                if (std::distance(m_frames.rbegin(), it) > 7)
                {
                    std::cerr << "...\n";
                    break;
                }
            }
        }
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
    }
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
    m_frames.back().push(std::move(value));
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
        Ark::logger.info("LOAD_SYMBOL ({0}) PP:{1}, IP:{2}"s, m_symbols[id], m_pp, m_ip);

    auto var = findNearestVariable(id);
    if (var != nullptr)
    {
        push(*var);
        m_last_sym_loaded = id;
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
        Ark::logger.info("LOAD_CONST ({0}) PP:{1}, IP:{2}"s, m_constants[id], m_pp, m_ip);
    
    if (m_saved_scope && m_constants[id].valueType() == ValueType::PageAddr)
    {
        push(Value(Closure(m_saved_scope.value(), m_constants[id].pageAddr())));
        m_saved_scope.reset();
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
        Ark::logger.info("STORE ({0}) PP:{1}, IP:{2}"s, m_symbols[id], m_pp, m_ip);

    auto var = findNearestVariable(id);
    if (var != nullptr)
    {
        if (var->isConst())
            throwVMError("can not modify a constant: " + m_symbols[id]);
        *var = pop();
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
        Ark::logger.info("LET ({0}) PP:{1}, IP:{2}"s, m_symbols[id], m_pp, m_ip);
    
    // check if we are redefining a variable
    if (getVariableInScope(id) != FFI::undefined)
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
    
    if (m_frames.back().stackSize() != 0)
    {
        Value return_value(pop());
        returnFromFuncCall();
        // push value as the return value of a function to the current stack
        push(return_value);
    }
    else
        returnFromFuncCall();
}

template<bool debug>
inline void VM_t<debug>::call(int16_t argc_)
{
    /*
        Argument: number of arguments when calling the function
        Job: Call function from its symbol id located on top of the stack. Take the given number of
                arguments from the top of stack and give them  to the function (the first argument taken
                from the stack will be the last one of the function). The stack of the function is now composed
                of its arguments, from the first to the last one
    */
    using namespace Ark::internal;

    uint16_t argc = 0;

    if (argc_ <= -1)
    {
        ++m_ip;
        argc = readNumber();
    }
    else
        argc = argc_;

    if constexpr (debug)
        Ark::logger.info("CALL ({0}) PP:{1}, IP:{2}"s, argc, m_pp, m_ip);

    Value function(pop());
    if constexpr (debug)
        Ark::logger.data("function object:", function);

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
            createNewScope();
            m_frames.emplace_back(m_ip, m_pp, new_page_pointer);
            // store "reference" to the function to speed the recursive functions
            registerVariable(m_last_sym_loaded, function);

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
            Closure& c = function.closure_ref();
            auto new_page_pointer = c.pageAddr();

            // load saved scope
            m_locals.push_back(c.scope());
            // create dedicated frame
            createNewScope();
            m_frames.back().incScopeCountToDelete();
            m_frames.emplace_back(m_ip, m_pp, new_page_pointer);

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
inline void VM_t<debug>::capture()
{
    /*
        Argument: symbol id (two bytes, big endian)
        Job: Used to tell the Virtual Machine to capture the variable from the current environment.
            Main goal is to be able to handle closures, which need to save the environment in which
            they were created
    */
    using namespace Ark::internal;

    ++m_ip;
    auto id = readNumber();

    if constexpr (debug)
        Ark::logger.info("CAPTURE ({0}) PP:{1}, IP:{2}"s, m_symbols[id], m_pp, m_ip);

    if (!m_saved_scope)
    {
        m_saved_scope = std::make_shared<std::vector<Value>>(
            m_symbols.size(), internal::FFI::undefined
        );
    }
    (*m_saved_scope.value())[id] = getVariableInScope(id);
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
        Ark::logger.info("BUILTIN ({0}) PP:{1}, IP:{2}"s, FFI::builtins[id].first, m_pp, m_ip);

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
        Ark::logger.info("MUT ({0}) PP:{1}, IP:{2}"s, m_symbols[id], m_pp, m_ip);

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
        Ark::logger.info("DEL ({0}) PP:{1}, IP:{2}"s, m_symbols[id], m_pp, m_ip);
    
    auto var = findNearestVariable(id);
    if (var != nullptr)
    {
        *var = FFI::undefined;
        return;
    }

    throwVMError("couldn't find symbol: " + m_symbols[id]);
}

template<bool debug>
inline void VM_t<debug>::saveEnv()
{
    /*
        Argument: none
        Job: Save the current environment, useful for quoted code
    */
    m_saved_scope = m_locals.back();
}

template<bool debug>
inline void VM_t<debug>::getField()
{
    /*
        Argument: symbol id (two bytes, big endian)
        Job: Used to read the field named following the given symbol id (cf symbols table) of a `Closure`
            stored in TS. Pop TS and push the value of field read on the stack
    */
    using namespace Ark::internal;

    ++m_ip;
    auto id = readNumber();

    if constexpr (debug)
        Ark::logger.info("GET_FIELD ({0}) PP:{1}, IP:{2}"s, m_symbols[id], m_pp, m_ip);
    
    auto var = pop();
    if (var.valueType() != ValueType::Closure)
        throwVMError("variable `" + m_symbols[m_last_sym_loaded] + "' isn't a closure, can not get the field `" + m_symbols[id] + "' from it");
    
    auto field = (*var.closure_ref().scope())[id];
    if (field != FFI::undefined)
    {
        if constexpr (debug)
            Ark::logger.data("Pushing closure field:", field);
        
        // check for CALL instruction
        if (m_ip + 1 < m_pages[m_pp].size() && m_pages[m_pp][m_ip + 1] == Instruction::CALL)
        {
            m_locals.push_back(var.closure_ref().scope());
            m_frames.back().incScopeCountToDelete();
        }

        push(field);
        return;
    }

    throwVMError("couldn't find symbol in closure enviroment: " + m_symbols[id]);
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
            if (a.valueType() == ValueType::Number)
            {
                if (b.valueType() != ValueType::Number)
                    throw Ark::TypeError("Arguments of + should have the same type");
                
                push(Value(a.number() + b.number()));
                break;
            }
            else if (a.valueType() == ValueType::String)
            {
                if (b.valueType() != ValueType::String)
                    throw Ark::TypeError("Arguments of + should have the same type");
                
                push(Value(a.string() + b.string()));
                break;
            }
            throw Ark::TypeError("Arguments of + should be Numbers or Strings");
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
            if (a.valueType() == ValueType::List)
                push((a.const_list().size() == 0) ? FFI::trueSym : FFI::falseSym);
            else if (a.valueType() == ValueType::String)
                push((a.string().size() == 0) ? FFI::trueSym : FFI::falseSym);
            else
                throw Ark::TypeError("Argument of empty? must be a list or a String");
            
            break;
        }

        case Instruction::FIRSTOF:
        {
            auto a = pop();
            if (a.valueType() == ValueType::List)
                push(a.const_list().size() > 0 ? a.const_list()[0] : FFI::nil);
            else if (a.valueType() == ValueType::String)
                push(a.string().size() > 0 ? Value(std::string(1, a.string()[0])) : FFI::nil);
            else
                throw Ark::TypeError("Argument of firstOf must be a list");

            break;
        }

        case Instruction::TAILOF:
        {
            auto a = pop();
            if (a.valueType() == ValueType::List)
            {
                if (a.const_list().size() < 2)
                {
                    push(FFI::nil);
                    break;
                }
                
                a.list().erase(a.const_list().begin());
                push(a);
            }
            else if (a.valueType() == ValueType::String)
            {
                if (a.string().size() < 2)
                {
                    push(FFI::nil);
                    break;
                }

                a.string_ref().erase(a.string().begin());
                push(a);
            }
            else
                throw Ark::TypeError("Argument of tailOf must be a list or a String");
            
            break;
        }

        case Instruction::HEADOF:
        {
            auto a = pop();
            if (a.valueType() == ValueType::List)
            {
                if (a.const_list().size() < 2)
                {
                    push(FFI::nil);
                    break;
                }
                
                a.list().pop_back();
                push(a);
            }
            else if (a.valueType() == ValueType::String)
            {
                if (a.string().size() < 2)
                {
                    push(FFI::nil);
                    break;
                }
                
                a.string_ref().pop_back();
                push(a);
            }
            else
                throw Ark::TypeError("Argument of headOf must be a list or a String");
            
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
            
            if (Utils::isDouble(a.string()))
                push(Value(std::stod(a.string().c_str())));
            else
                push(FFI::nil);
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
            if (b.valueType() != ValueType::Number)
                throw Ark::TypeError("Argument 2 of @ should be a Number");

            if (a.valueType() == ValueType::List)
                push(a.const_list()[static_cast<long>(b.number())]);
            else if (a.valueType() == ValueType::String)
                push(Value(std::string(1, a.string()[static_cast<long>(b.number())])));
            else
                throw Ark::TypeError("Argument 1 of @ should be a List or a String");
            break;
        }

        case Instruction::AND_:
        {
            auto a = pop(), b = pop();
            push((a == FFI::trueSym && b == FFI::trueSym) ? FFI::trueSym : FFI::falseSym);
            break;
        }

        case Instruction::OR_:
        {
            auto a = pop(), b = pop();
            push((b == FFI::trueSym || a == FFI::trueSym) ? FFI::trueSym : FFI::falseSym);
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

        case Instruction::TYPE:
        {
            auto a = pop();
            switch (a.valueType())
            {
                case ValueType::List:     push(Value("List"));     break;
                case ValueType::Number:   push(Value("Number"));   break;
                case ValueType::String:   push(Value("String"));   break;
                case ValueType::PageAddr: push(Value("Function")); break;
                case ValueType::NFT:
                {
                    switch (a.nft())
                    {
                        case NFT::Nil:
                            push(Value("Nil"));
                            break;
                        case NFT::True:
                        case NFT::False:
                            push(Value("Bool"));
                            break;
                        case NFT::Undefined:
                            push(Value("Undefined"));
                            break;
                    }
                    break;
                }
                case ValueType::CProc:   push(Value("CProc"));   break;
                case ValueType::Closure: push(Value("Closure")); break;
                default:
                    throw Ark::TypeError("unimplemented type");
            }
            break;
        }

        case Instruction::HASFIELD:
        {
            auto field = pop(), closure = pop();
            if (closure.valueType() != ValueType::Closure)
                throw Ark::TypeError("Argument no 1 of hasField should be a Closure");
            if (field.valueType() != ValueType::String)
                throw Ark::TypeError("Argument no 2 of hasField should be a String");
            
            auto it = std::find(m_symbols.begin(), m_symbols.end(), field.string());
            if (it == m_symbols.end())
            {
                push(FFI::falseSym);
                break;
            }
            auto id = static_cast<uint16_t>(std::distance(m_symbols.begin(), it));
            
            if ((*closure.closure_ref().scope_ref())[id] != FFI::undefined)
                push(FFI::trueSym);
            else
                push(FFI::falseSym);
            
            break;
        }
    }
}