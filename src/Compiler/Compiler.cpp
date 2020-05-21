#include <Ark/Compiler/Compiler.hpp>

#include <fstream>
#include <chrono>

#include <Ark/Log.hpp>
#include <Ark/Builtins/Builtins.hpp>

namespace Ark
{
    using namespace Ark::internal;

    Compiler::Compiler(unsigned debug, const std::string& lib_dir, uint16_t options) :
        m_parser(debug, lib_dir, options), m_options(options), m_debug(debug)
    {}

    void Compiler::feed(const std::string& code, const std::string& filename)
    {
        m_parser.feed(code, filename);

        if (m_debug >= 2)
        {
            Ark::logger.info(filename + " is importing " + Ark::Utils::toString(m_parser.getImports().size()) + " files:");
            for (auto&& import: m_parser.getImports())
                Ark::logger.data("\t" + import);
        }
    }

    void Compiler::compile()
    {
        /*
            Generating headers:
                - lang name (to be sure we are executing an ArkScript file)
                    on 4 bytes (ark + padding)
                - version (major: 2 bytes, minor: 2 bytes, patch: 2 bytes)
                - timestamp (8 bytes, unix format)
                - symbols table header
                    + elements
                - values table header
                    + elements
        */

        if (m_debug >= 2)
            Ark::logger.info("Adding magic constant");

        m_bytecode.push_back('a');
        m_bytecode.push_back('r');
        m_bytecode.push_back('k');
        m_bytecode.push_back(Instruction::NOP);

        // push version
        if (m_debug >= 2)
        {
            Ark::logger.info("Major: ", ARK_VERSION_MAJOR);
            Ark::logger.info("Minor: ", ARK_VERSION_MINOR);
            Ark::logger.info("Patch: ", ARK_VERSION_PATCH);
        }
        pushNumber(ARK_VERSION_MAJOR);
        pushNumber(ARK_VERSION_MINOR);
        pushNumber(ARK_VERSION_PATCH);

        // push timestamp
        unsigned long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        for (char c=0; c < 8; c++)
        {
            unsigned d = 56 - 8 * c;
            uint8_t b = (timestamp & (0xff << d)) >> d;
            m_bytecode.push_back(b);
            if (m_debug >= 2)
                std::cout << static_cast<int>(b) << " ";
        }
        if (m_debug >= 2)
            std::cout << std::endl;
        
        if (m_debug >= 1)
            Ark::logger.info("Timestamp: ", timestamp);

        if (m_debug >= 1)
            Ark::logger.info("Adding symbols table header");

        // symbols table
        m_bytecode.push_back(Instruction::SYM_TABLE_START);
            if (m_debug >= 1)
                Ark::logger.info("Compiling");
            // gather symbols, values, and start to create code segments
            m_code_pages.emplace_back();  // create empty page
            _compile(m_parser.ast(), 0);
        if (m_debug >= 1)
            Ark::logger.info("Adding symbols table");
        // push size
        pushNumber(static_cast<uint16_t>(m_symbols.size()));
        // push elements
        for (auto sym : m_symbols)
        {
            // push the string, null terminated
            for (std::size_t i=0, size=sym.size(); i < size; ++i)
                m_bytecode.push_back(sym[i]);
            m_bytecode.push_back(Instruction::NOP);
        }

        if (m_debug >= 1)
            Ark::logger.info("Adding constants table");

        // values table
        m_bytecode.push_back(Instruction::VAL_TABLE_START);
        // push size
        pushNumber(static_cast<uint16_t>(m_values.size()));
        // push elements (separated with 0x00)
        for (auto val : m_values)
        {
            if (val.type == CValueType::Number)
            {
                m_bytecode.push_back(Instruction::NUMBER_TYPE);
                auto n = std::get<double>(val.value);
                std::string t = Ark::Utils::toString(n);
                for (std::size_t i=0, size=t.size(); i < size; ++i)
                    m_bytecode.push_back(t[i]);
            }
            else if (val.type == CValueType::String)
            {
                m_bytecode.push_back(Instruction::STRING_TYPE);
                std::string t = std::get<std::string>(val.value);
                for (std::size_t i=0, size=t.size(); i < size; ++i)
                    m_bytecode.push_back(t[i]);
            }
            else if (val.type == CValueType::PageAddr)
            {
                m_bytecode.push_back(Instruction::FUNC_TYPE);
                pushNumber(static_cast<uint16_t>(std::get<std::size_t>(val.value)));
            }
            else
            {
                throw std::runtime_error("CompilerError: trying to put a value in the value table, but the type isn't handled.\nCertainly a logic problem in the compiler source code");
            }

            m_bytecode.push_back(Instruction::NOP);
        }

        if (m_debug >= 1)
            Ark::logger.info("Adding plugins table");
        
        // plugins table
        m_bytecode.push_back(Instruction::PLUGIN_TABLE_START);
        // push size
        pushNumber(static_cast<uint16_t>(m_plugins.size()));
        // push elements
        for (auto plugin: m_plugins)
        {
            // push the string, null terminated
            for (std::size_t i=0, size=plugin.size(); i < size; ++i)
                m_bytecode.push_back(plugin[i]);
            m_bytecode.push_back(Instruction::NOP);
        }

        if (m_debug >= 1)
            Ark::logger.info("Adding code segments");

        // start code segments
        for (auto page : m_code_pages)
        {
            if (m_debug >= 2)
                Ark::logger.info("-", page.size() + 1);

            m_bytecode.push_back(Instruction::CODE_SEGMENT_START);
            // push number of elements
            if (!page.size())
            {
                pushNumber(0x01);
                m_bytecode.push_back(Instruction::HALT);
                return;
            }
            pushNumber(static_cast<uint16_t>(page.size() + 1));

            for (auto inst : page)
                m_bytecode.push_back(inst.inst);
            // just in case we got too far, always add a HALT to be sure the
            // VM won't do anything crazy
            m_bytecode.push_back(Instruction::HALT);
        }

        if (!m_code_pages.size())
        {
            m_bytecode.push_back(Instruction::CODE_SEGMENT_START);
            pushNumber(static_cast<uint16_t>(1));
            m_bytecode.push_back(Instruction::HALT);
        }
    }

    void Compiler::saveTo(const std::string& file)
    {
        if (m_debug >= 1)
            Ark::logger.info("Final bytecode size:", m_bytecode.size() * sizeof(uint8_t), "B");

        std::ofstream output(file, std::ofstream::binary);
        output.write(reinterpret_cast<char*>(&m_bytecode[0]), m_bytecode.size() * sizeof(uint8_t));
        output.close();
    }

    const bytecode_t& Compiler::bytecode()
    {
        return m_bytecode;
    }

    void Compiler::_compile(const Ark::internal::Node& x, int p)
    {
        if (m_debug >= 2)
            Ark::logger.info(x);

        // register symbols
        if (x.nodeType() == Ark::internal::NodeType::Symbol)
        {
            std::string name = x.string();

            // check if 'name' isn't a builtin/operator name before pushing it as a 'var-use'
            if (auto it_builtin = isBuiltin(name))
            {
                page(p).emplace_back(Instruction::BUILTIN);
                pushNumber(static_cast<uint16_t>(it_builtin.value()), &page(p));
            }
            else if (auto it_operator = isOperator(name))
            {
                page(p).emplace_back(static_cast<Instruction>(Instruction::FIRST_OPERATOR + it_operator.value()));
            }
            else
            {
                std::size_t i = addSymbol(name);

                page(p).emplace_back(Instruction::LOAD_SYMBOL);
                pushNumber(static_cast<uint16_t>(i), &page(p));
            }

            return;
        }
        if (x.nodeType() == Ark::internal::NodeType::GetField)
        {
            std::string name = x.string();
            // 'name' shouldn't be a builtin/operator, we can use it as-is
            std::size_t i = addSymbol(name);

            page(p).emplace_back(Instruction::GET_FIELD);
            pushNumber(static_cast<uint16_t>(i), &page(p));

            return;
        }
        // register values
        if (x.nodeType() == Ark::internal::NodeType::String || x.nodeType() == Ark::internal::NodeType::Number)
        {
            std::size_t i = addValue(x);

            page(p).emplace_back(Instruction::LOAD_CONST);
            pushNumber(static_cast<uint16_t>(i), &page(p));

            return;
        }
        // empty code block should be nil
        if (x.const_list().empty())
        {
            auto it_builtin = isBuiltin("nil");
            page(p).emplace_back(Instruction::BUILTIN);
            pushNumber(static_cast<uint16_t>(it_builtin.value()), &page(p));
            return;
        }
        // registering structures
        if (x.const_list()[0].nodeType() == Ark::internal::NodeType::Keyword)
        {
            Ark::internal::Keyword n = x.const_list()[0].keyword();

            if (n == Ark::internal::Keyword::If)
            {
                // compile condition
                _compile(x.const_list()[1], p);
                // jump only if needed to the x.list()[2] part
                page(p).emplace_back(Instruction::POP_JUMP_IF_TRUE);
                std::size_t jump_to_if_pos = page(p).size();
                // absolute address to jump to if condition is true
                pushNumber(static_cast<uint16_t>(0x00), &page(p));
                    // else code
                    _compile(x.const_list()[3], p);
                    // when else is finished, jump to end
                    page(p).emplace_back(Instruction::JUMP);
                    std::size_t jump_to_end_pos = page(p).size();
                    pushNumber(static_cast<uint16_t>(0x00), &page(p));
                // set jump to if pos
                page(p)[jump_to_if_pos]     = (static_cast<uint16_t>(page(p).size()) & 0xff00) >> 8;
                page(p)[jump_to_if_pos + 1] =  static_cast<uint16_t>(page(p).size()) & 0x00ff;
                // if code
                _compile(x.const_list()[2], p);
                // set jump to end pos
                page(p)[jump_to_end_pos]     = (static_cast<uint16_t>(page(p).size()) & 0xff00) >> 8;
                page(p)[jump_to_end_pos + 1] =  static_cast<uint16_t>(page(p).size()) & 0x00ff;
            }
            else if (n == Ark::internal::Keyword::Set)
            {
                std::string name = x.const_list()[1].string();
                std::size_t i = addSymbol(name);

                // put value before symbol id
                _compile(x.const_list()[2], p);

                page(p).emplace_back(Instruction::STORE);
                pushNumber(static_cast<uint16_t>(i), &page(p));
            }
            else if (n == Ark::internal::Keyword::Let)
            {
                std::string name = x.const_list()[1].string();
                std::size_t i = addSymbol(name);

                // put value before symbol id
                _compile(x.const_list()[2], p);

                page(p).emplace_back(Instruction::LET);
                pushNumber(static_cast<uint16_t>(i), &page(p));
            }
            else if (n == Ark::internal::Keyword::Mut)
            {
                std::string name = x.const_list()[1].string();
                std::size_t i = addSymbol(name);

                // put value before symbol id
                _compile(x.const_list()[2], p);

                page(p).emplace_back(Instruction::MUT);
                pushNumber(static_cast<uint16_t>(i), &page(p));
            }
            else if (n == Ark::internal::Keyword::Fun)
            {
                // capture, if needed
                for (auto it=x.const_list()[1].const_list().begin(), it_end=x.const_list()[1].const_list().end(); it != it_end; ++it)
                {
                    if (it->nodeType() == NodeType::Capture)
                    {
                        page(p).emplace_back(Instruction::CAPTURE);
                        std::size_t var_id = addSymbol(it->string());
                        pushNumber(static_cast<uint16_t>(var_id), &(page(p)));
                    }
                }
                // create new page for function body
                m_code_pages.emplace_back();
                std::size_t page_id = m_code_pages.size() - 1;
                // load value on the stack
                page(p).emplace_back(Instruction::LOAD_CONST);
                std::size_t id = addValue(page_id);  // save page_id into the constants table as PageAddr
                pushNumber(static_cast<uint16_t>(id), &page(p));
                // pushing arguments from the stack into variables in the new scope
                for (auto it=x.const_list()[1].const_list().begin(), it_end=x.const_list()[1].const_list().end(); it != it_end; ++it)
                {
                    if (it->nodeType() == NodeType::Symbol)
                    {
                        page(page_id).emplace_back(Instruction::MUT);
                        std::size_t var_id = addSymbol(it->string());
                        pushNumber(static_cast<uint16_t>(var_id), &(page(page_id)));
                    }
                }
                // push body of the function
                _compile(x.const_list()[2], page_id);
                // return last value on the stack
                page(page_id).emplace_back(Instruction::RET);
            }
            else if (n == Ark::internal::Keyword::Begin)
            {
                for (std::size_t i=1, size=x.const_list().size(); i < size; ++i)
                    _compile(x.const_list()[i], p);
            }
            else if (n == Ark::internal::Keyword::While)
            {
                // save current position to jump there at the end of the loop
                std::size_t current = page(p).size();
                // push condition
                _compile(x.const_list()[1], p);
                // absolute jump to end of block if condition is false
                page(p).emplace_back(Instruction::POP_JUMP_IF_FALSE);
                std::size_t jump_to_end_pos = page(p).size();
                // absolute address to jump to if condition is false
                pushNumber(static_cast<uint16_t>(0x00), &page(p));
                // push code to page
                    _compile(x.const_list()[2], p);
                    // loop, jump to the condition
                    page(p).emplace_back(Instruction::JUMP);
                    // abosolute address
                    pushNumber(static_cast<uint16_t>(current), &page(p));
                // set jump to end pos
                page(p)[jump_to_end_pos]     = (static_cast<uint16_t>(page(p).size()) & 0xff00) >> 8;
                page(p)[jump_to_end_pos + 1] =  static_cast<uint16_t>(page(p).size()) & 0x00ff;
            }
            else if (n == Ark::internal::Keyword::Import)
            {
                for (auto it=x.const_list().begin() + 1, it_end=x.const_list().end(); it != it_end; ++it)
                {
                    // load const, push it to the plugins table
                    addPlugin(*it);
                }
            }
            else if (n == Ark::internal::Keyword::Quote)
            {
                // create new page for quoted code
                m_code_pages.emplace_back();
                std::size_t page_id = m_code_pages.size() - 1;
                _compile(x.const_list()[1], page_id);
                page(page_id).emplace_back(Instruction::RET);  // return to the last frame

                // call it
                std::size_t id = addValue(page_id);  // save page_id into the constants table as PageAddr
                // page(p).emplace_back(Instruction::SAVE_ENV);
                page(p).emplace_back(Instruction::LOAD_CONST);
                pushNumber(static_cast<uint16_t>(id), &page(p));
            }
            else if (n == Ark::internal::Keyword::Del)
            {
                // get id of symbol to delete
                std::string name = x.const_list()[1].string();
                std::size_t i = addSymbol(name);

                page(p).emplace_back(Instruction::DEL);
                pushNumber(static_cast<uint16_t>(i), &page(p));
            }

            return;
        }

        // if we are here, we should have a function name
        // push arguments first, then function name, then call it
            m_temp_pages.emplace_back();
            int proc_page = -static_cast<int>(m_temp_pages.size());
            _compile(x.const_list()[0], proc_page);  // storing proc
            // trying to handle chained closure.field.field.field...
                std::size_t n = 1;
                while (n < x.const_list().size())
                {
                    if (x.const_list()[n].nodeType() == Ark::internal::NodeType::GetField)
                    {
                        _compile(x.const_list()[n], proc_page);
                        n++;
                    }
                    else
                        break;
                }
            std::size_t proc_page_len = m_temp_pages.back().size();
        // we know that operators take only 1 instruction, so if there are more
        // it's a builtin/function
        if (proc_page_len > 1)
        {
            // push arguments on current page
            for (auto exp=x.const_list().begin() + n, exp_end=x.const_list().end(); exp != exp_end; ++exp)
                _compile(*exp, p);
            // push proc from temp page
            for (auto&& inst : m_temp_pages.back())
                page(p).push_back(inst);
            m_temp_pages.pop_back();

            // call the procedure
            page(p).push_back(Instruction::CALL);
            // number of arguments
            std::size_t args_count = 0;
            for (auto it=x.const_list().begin() + 1, it_end=x.const_list().end(); it != it_end; ++it)
            {
                if (it->nodeType() != Ark::internal::NodeType::GetField &&
                    it->nodeType() != Ark::internal::NodeType::Capture)
                    args_count++;
            }
            pushNumber(static_cast<uint16_t>(args_count), &page(p));
        }
        else  // operator
        {
            // retrieve operator
            auto op_inst = m_temp_pages.back()[0];
            m_temp_pages.pop_back();

            // push arguments on current page
            std::size_t exp_count = 0;
            for (std::size_t index=n, size=x.const_list().size(); index < size; ++index)
            {
                _compile(x.const_list()[index], p);

                if ((index + 1 < size &&
                    x.const_list()[index + 1].nodeType() != Ark::internal::NodeType::GetField &&
                    x.const_list()[index + 1].nodeType() != Ark::internal::NodeType::Capture) ||
                    index + 1 == size)
                    exp_count++;

                // in order to be able to handle things like (op A B C D...)
                // which should be transformed into A B op C op D op...
                if (exp_count >= 2)
                    page(p).push_back(op_inst);
            }

            if (exp_count == 1)
                page(p).push_back(op_inst);

            // need to check we didn't push the (op A B C D...) things for operators not supporting it
            if (exp_count > 2)
            {
                switch (op_inst.inst)
                {
                    // authorized instructions
                    case Instruction::ADD:
                    case Instruction::SUB:
                    case Instruction::DIV:
                    case Instruction::MUL:
                    case Instruction::MOD:
                    case Instruction::AND_:
                    case Instruction::OR_:
                        break;
                    
                    default:
                        throw std::runtime_error("CompilerError: can not create a chained expression (of length " + Utils::toString(exp_count) +
                            ") for operator `" + Builtins::operators[static_cast<std::size_t>(op_inst.inst - Instruction::FIRST_OPERATOR)] + "' " +
                            "at node `" + Utils::toString(x) + "'");
                }
            }
        }

        return;
    }

    std::size_t Compiler::addSymbol(const std::string& sym)
    {
        // otherwise, add the symbol, and return its id in the table
        auto it = std::find(m_symbols.begin(), m_symbols.end(), sym);
        if (it == m_symbols.end())
        {
            if (m_debug >= 2)
                Ark::logger.info("Registering symbol:", sym, "(", m_symbols.size(), ")");

            m_symbols.push_back(sym);
            return m_symbols.size() - 1;
        }
        return static_cast<std::size_t>(std::distance(m_symbols.begin(), it));
    }

    std::size_t Compiler::addValue(const Ark::internal::Node& x)
    {
        CValue v(x);
        auto it = std::find(m_values.begin(), m_values.end(), v);
        if (it == m_values.end())
        {
            if (m_debug >= 2)
                Ark::logger.info("Registering value (", m_values.size(), ")");
            
            m_values.push_back(v);
            return m_values.size() - 1;
        }
        return static_cast<std::size_t>(std::distance(m_values.begin(), it));
    }

    std::size_t Compiler::addValue(std::size_t page_id)
    {
        CValue v(page_id);
        auto it = std::find(m_values.begin(), m_values.end(), v);
        if (it == m_values.end())
        {
            if (m_debug >= 2)
                Ark::logger.info("Registering value (", m_values.size(), ")");
            
            m_values.push_back(v);
            return m_values.size() - 1;
        }
        return static_cast<std::size_t>(std::distance(m_values.begin(), it));
    }

    void Compiler::addPlugin(const Ark::internal::Node& x)
    {
        std::string name = x.string();
        if (std::find(m_plugins.begin(), m_plugins.end(), name) == m_plugins.end())
            m_plugins.push_back(name);
    }

    void Compiler::pushNumber(uint16_t n, std::vector<Inst>* page)
    {
        if (page == nullptr)
        {
            m_bytecode.push_back((n & 0xff00) >> 8);
            m_bytecode.push_back(n & 0x00ff);
        }
        else
        {
            page->emplace_back((n & 0xff00) >> 8);
            page->emplace_back(n & 0x00ff);
        }
    }
}