#include <Ark/VM/VM.hpp>

#include <exception>
#include <stdexcept>
#include <filesystem>

#include <Ark/Log.hpp>
#include <Ark/VM/FFI.hpp>
#include <Ark/Utils.hpp>

namespace Ark
{
    namespace VM
    {
        using namespace std::string_literals;

        VM::VM(bool debug) :
            m_debug(debug),
            m_ip(0), m_pp(0),
            m_running(false),
            m_filename("FILE")
        {}
        
        void VM::feed(const std::string& filename)
        {
            Ark::Compiler::BytecodeReader bcr;
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
                        {
                            Ark::logger.error("[Virtual Machine] Page pointer has gone too far");
                            exit(1);
                        }
                        if (m_ip >= m_pages[m_pp].size())
                        {
                            Ark::logger.error("[Virtual Machine] Instruction pointer has gone too far");
                            exit(1);
                        }

                        // get current instruction
                        uint8_t inst = m_pages[m_pp][m_ip];

                        // and it's time to du-du-du-du-duel!
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
                        
                        case Instruction::NEW_ENV:
                            newEnv();
                            break;
                        
                        case Instruction::BUILTIN:
                            builtin();
                            break;
                        
                        case Instruction::SAVE_ENV:
                            saveEnv();
                            break;
                        
                        default:
                            Ark::logger.error("[Virtual Machine] unknown instruction:", static_cast<std::size_t>(inst));
                            exit(1);
                        }

                        // move forward
                        ++m_ip;
                    }
                }
                catch (const std::exception& e)
                {
                    Ark::logger.error(e.what());
                }
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

            m_frames.front()[static_cast<uint16_t>(std::distance(m_symbols.begin(), it))] = Value(function);
        }

        void VM::configure()
        {
            // configure ffi
            m_frames.emplace_back();  // put default page
            if (m_ffi.size() == 0)
                initFFI();

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
            {
                Ark::logger.error("[Virtual Machine] invalid format: couldn't find magic constant");
                exit(1);
            }

            if (m_debug)
                Ark::logger.info("(Virtual Machine) magic constant found: ark\\0");

            if (b[i] == Instruction::SYM_TABLE_START)
            {
                if (m_debug)
                    Ark::logger.info("(Virtual Machine) symbols table");
                
                i++;
                uint16_t size = readNumber(i);
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
                m_symbols.push_back(".c");
                m_dotc_idx = m_symbols.size() - 1;
            }
            else
            {
                Ark::logger.error("[Virtual Machine] Couldn't find symbols table");
                exit(1);
            }

            if (b[i] == Instruction::VAL_TABLE_START)
            {
                if (m_debug)
                    Ark::logger.info("(Virtual Machine) constants table");
                
                i++;
                uint16_t size = readNumber(i);
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

                        m_constants.emplace_back(BigNum(val, /* base */ 16));
                        
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
                    {
                        if (m_debug)
                            Ark::logger.info("(Virtual Machine) Unknown value type");
                        return;
                    }
                }
            }
            else
            {
                Ark::logger.error("[Virtual Machine] Couldn't find constants table");
                exit(1);
            }

            if (b[i] == Instruction::PLUGIN_TABLE_START)
            {
                if (m_debug)
                    Ark::logger.info("(Virtual Machine) plugins table");
                
                i++;
                uint16_t size = readNumber(i);
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
            {
                Ark::logger.error("[Virtual Machine] Couldn't find plugins table");
                exit(1);
            }
            
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

                for (uint16_t j=0; j < size; ++j)
                    m_pages.back().push_back(b[i++]);
            }

            // loading plugins
            for (const auto& file: m_plugins)
            {
                namespace fs = std::filesystem;

                std::string path = "./" + file;
                if (m_filename != "FILE")  // bytecode loaded from file
                    path = (fs::path(m_filename).root_path() / fs::path(file)).string();
                std::string lib_path = (fs::path(ARK_STD) / fs::path(file)).string();

                if (Ark::Utils::fileExists(path))  // if it exists alongside the .arkc file
                    m_shared_lib_objects.emplace_back(path);
                else if (Ark::Utils::fileExists(lib_path))  // check in LOAD_PATH otherwise
                    m_shared_lib_objects.emplace_back(lib_path);
                else
                {
                    Ark::logger.error("[Virtual Machine] Could not load plugin", file);
                    exit(1);
                }

                // load data from it!
                // m_shared_lib_objects.back().get<T>("name")(args...);
            }
        }

        void VM::initFFI()
        {
            #define FFI_VM
            #define FFI_INIT_VM_FFI

            #include <Ark/MakeFFI.hpp>

            #undef FFI_VM
            #undef FFI_INIT_VM_FFI
        }

        Value VM::pop()
        {
            return m_frames.back().pop();
        }

        void VM::push(const Value& value)
        {
            m_frames.back().push(value);
        }

        void VM::nop()
        {
            // Does nothing
            if (m_debug)
                Ark::logger.info("NOP PP:{0}, IP:{1}"s, m_pp, m_ip);
        }
        
        void VM::loadSymbol()
        {
            /*
                Argument: symbol id (two bytes, big endian)
                Job: Load a symbol from its id onto the stack
            */
            ++m_ip;
            auto id = readNumber();

            if (m_debug)
                Ark::logger.info("LOAD_SYMBOL ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

            if (id == 0)
                push(NFT::Nil);
            else if (id == 1)
                push(NFT::False);
            else if (id == 2)
                push(NFT::True);
            else
            {
                auto sid = id - 3;
                std::string sym = m_symbols[sid];

                for (std::size_t i=m_frames.size() - 1; ; --i)
                {
                    if (m_frames[i].find(sid))
                    {
                        push(m_frames[i][sid]);
                        return;
                    }

                    if (i == 0)
                        break;
                }

                Ark::logger.error("[Virtual Machine] Couldn't find symbol to load:", sym);
                exit(1);
            }
        }
        
        void VM::loadConst()
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
            
            if (m_saved_frame && m_constants[id].isPageAddr())
            {
                push(Value(&m_saved_frame.value(), m_constants[id].pageAddr()));
                m_saved_frame.reset();
            }
            else
                push(m_constants[id]);
        }
        
        void VM::popJumpIfTrue()
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

            Value cond = pop();
            if (cond.isNFT() && cond.nft() == NFT::True)
                m_ip = addr - 1;  // because we are doing a ++m_ip right after this
        }
        
        void VM::store()
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

            auto sid = id - 3;
            std::string sym = m_symbols[sid];

            for (std::size_t i=m_frames.size() - 1; ; --i)
            {
                if (m_frames[i].find(sid))
                {
                    m_frames[i][sid] = pop();
                    if (m_frames[i][sid].isClosure())
                        m_frames[i][sid].closure_ref().save(i, sid);
                    return;
                }

                if (i == 0)
                    break;
            }

            Ark::logger.error("[Virtual Machine] Couldn't find symbol:", sym);
            exit(1);
        }
        
        void VM::let()
        {
            /*
                Argument: symbol id (two bytes, big endian)
                Job: Take the value on top of the stack and create a variable in the current scope, named
                     following the given symbol id (cf symbols table)
            */
            ++m_ip;
            auto id = readNumber();

            if (m_debug)
                Ark::logger.info("LET ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

            auto sid = id - 3;
            std::string sym = m_symbols[sid];
            m_frames.back()[sid] = pop();
            if (m_frames.back()[sid].isClosure())
                m_frames.back()[sid].closure_ref().save(m_frames.size() - 1, sid);
        }
        
        void VM::popJumpIfFalse()
        {
            /*
                Argument: relative address to jump to (two bytes, big endian)
                Job: Jump to the provided address if the last value on the stack was equal to false. Remove
                     the value from the stack no matter what it is
            */
            ++m_ip;
            int16_t addr = static_cast<int16_t>(readNumber());

            if (m_debug)
                Ark::logger.info("POP_JUMP_IF_FALSE ({0}) PP:{1}, IP:{2}"s, addr, m_pp, m_ip);

            Value cond = pop();
            if (cond.isNFT() && cond.nft() == NFT::False)
                m_ip += addr - 1;  // because we are doing a ++m_ip right after this
        }
        
        void VM::jump()
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
        
        void VM::ret()
        {
            /*
                Argument: none
                Job: If in a code segment other than the main one, quit it, and push the value on top of
                     the stack to the new stack ; should as well delete the current environment.
                     Otherwise, acts as a `HALT`
            */
            // check if we should halt the VM
            if (m_debug)
                Ark::logger.info("RET PP:{0}, IP:{1}"s, m_pp, m_ip);
            
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
                Closure c = m_frames.back()[m_dotc_idx].closure_ref();
                // remove frame
                m_frames.pop_back();
                // next frame is the one of the closure, we should save it
                m_frames.back().copyEnvironmentTo(*c.frame());
                // remove it
                m_frames.pop_back();

                m_frames[c.frameIndex()][c.symbol()] = Value(c);
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
        
        void VM::call()
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

            Value function(pop());
            std::vector<Value> args;
            for (uint16_t j=0; j < argc; ++j)
                args.push_back(pop());

            // is it a builtin function name?
            if (function.isProc())
            {
                // reverse arguments
                std::reverse(args.begin(), args.end());
                // call proc
                Value return_value = function.proc()(args);
                push(return_value);
                return;
            }
            else if (function.isClosure())
            {
                Closure c = function.closure();
                // load saved frame
                m_frames.push_back(*c.frame());
                // create dedicated frame
                m_frames.emplace_back(m_ip, m_pp);
                    m_frames.back()[m_dotc_idx] = function;
                m_pp = c.pageAddr();
                m_ip = -1;  // because we are doing a m_ip++ right after that
                for (std::size_t j=0; j < args.size(); ++j)
                    push(args[j]);
                return;
            }

            Ark::logger.error("[Virtual Machine] Couldn't identify function object");
            exit(1);
        }
        
        void VM::newEnv()
        {
            /*
                Argument: none
                Job: Create a new environment (a new scope for variables) in the Virtual Machine
            */
            if (m_debug)
                Ark::logger.info("NEW_ENV PP:{0}, IP:{1}"s, m_pp, m_ip);
            
            m_frames.emplace_back();
        }

        void VM::builtin()
        {
            /*
                Argument: id of builtin (two bytes, big endian)
                Job: Push the builtin function object on the stack
            */
            ++m_ip;
            auto id = readNumber();

            if (m_debug)
                Ark::logger.info("BUILTIN ({0}) PP:{1}, IP:{2}"s, id, m_pp, m_ip);

            push(m_ffi[id]);
        }

        void VM::saveEnv()
        {
            /*
                Argument: none
                Job: Used to tell the Virtual Machine to save the current environment. Main goal is
                     to be able to handle closures, which need to save the environment in which
                     they were created
            */
            m_saved_frame = m_frames.back();
        }
    }
}