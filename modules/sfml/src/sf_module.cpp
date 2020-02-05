#include <SFML/Graphics.hpp>
#include <Ark/Module.hpp>

bool& get_has_window() { static bool w = false; return w; }
sf::RenderWindow& get_window() { static sf::RenderWindow w; return w; }
sf::Event& get_event() { static sf::Event e; return e; }
std::unordered_map<std::string, sf::Texture>& get_textures() { static std::unordered_map<std::string, sf::Texture> t; return t; }
std::unordered_map<std::string, sf::Sprite>& get_sprites() { static std::unordered_map<std::string, sf::Sprite> s; return s; }
std::unordered_map<std::string, sf::Font>& get_fonts() { static std::unordered_map<std::string, sf::Font> f; return f; }
std::unordered_map<std::string, sf::Text>& get_texts() { static std::unordered_map<std::string, sf::Text> t; return t; }

// module functions
Value sf_window_init(const std::vector<Value>& n)
{
    if (n.size() != 3)
        throw std::runtime_error("sfWindowInit needs 3 arguments: width, height and title");
    if (n[0].valueType() != ValueType::Number)
        throw Ark::TypeError("sfWindowInit: width must be a Number");
    if (n[1].valueType() != ValueType::Number)
        throw Ark::TypeError("sfWindowInit: height must be a Number");
    if (n[2].valueType() != ValueType::String)
        throw Ark::TypeError("sfWindowInit: title must be a String");
    
    if (!get_has_window())
    {
        get_window().create(sf::VideoMode(static_cast<long>(n[0].number()), static_cast<long>(n[1].number())), n[2].string());
        get_has_window() = true;
    }
    else
        throw std::runtime_error("sfWindowInit: can't call the function twice");

    return Nil;
}

Value sf_window_isopen(const std::vector<Value>& n)
{
    return get_window().isOpen() ? True : False;
}

Value sf_poll_event(const std::vector<Value>& n)
{
    if (get_window().pollEvent(get_event()))
    {
        std::string out = "event-";

        if (get_event().type == sf::Event::Closed)
            out += "quit";
        else if (get_event().type == sf::Event::KeyReleased)
            out += "keyup";
        else if (get_event().type == sf::Event::KeyPressed)
            out += "keydown";
        else
            out += "unknown";
        
        if (get_event().type == sf::Event::KeyReleased || get_event().type == sf::Event::KeyPressed)
        {
            switch (get_event().key.code)
            {
            case sf::Keyboard::A: out += "a"; break;
            case sf::Keyboard::B: out += "b"; break;
            case sf::Keyboard::C: out += "c"; break;
            case sf::Keyboard::D: out += "d"; break;
            case sf::Keyboard::E: out += "e"; break;
            case sf::Keyboard::F: out += "f"; break;
            case sf::Keyboard::G: out += "g"; break;
            case sf::Keyboard::H: out += "h"; break;
            case sf::Keyboard::I: out += "i"; break;
            case sf::Keyboard::J: out += "j"; break;
            case sf::Keyboard::K: out += "k"; break;
            case sf::Keyboard::L: out += "l"; break;
            case sf::Keyboard::M: out += "m"; break;
            case sf::Keyboard::N: out += "n"; break;
            case sf::Keyboard::O: out += "o"; break;
            case sf::Keyboard::P: out += "p"; break;
            case sf::Keyboard::Q: out += "q"; break;
            case sf::Keyboard::R: out += "r"; break;
            case sf::Keyboard::S: out += "s"; break;
            case sf::Keyboard::T: out += "t"; break;
            case sf::Keyboard::U: out += "u"; break;
            case sf::Keyboard::V: out += "v"; break;
            case sf::Keyboard::W: out += "w"; break;
            case sf::Keyboard::X: out += "x"; break;
            case sf::Keyboard::Y: out += "y"; break;
            case sf::Keyboard::Z: out += "z"; break;
            case sf::Keyboard::Num0: out += "0"; break;
            case sf::Keyboard::Num1: out += "1"; break;
            case sf::Keyboard::Num2: out += "2"; break;
            case sf::Keyboard::Num3: out += "3"; break;
            case sf::Keyboard::Num4: out += "4"; break;
            case sf::Keyboard::Num5: out += "5"; break;
            case sf::Keyboard::Num6: out += "6"; break;
            case sf::Keyboard::Num7: out += "7"; break;
            case sf::Keyboard::Num8: out += "8"; break;
            case sf::Keyboard::Num9: out += "9"; break;
            case sf::Keyboard::Escape:   out += "escape"; break;
            case sf::Keyboard::LBracket: out += "["; break;
            case sf::Keyboard::RBracket: out += "]"; break;
            case sf::Keyboard::Semicolon: out += ";"; break;
            case sf::Keyboard::Comma:    out += ","; break;
            case sf::Keyboard::Period:   out += "."; break;
            case sf::Keyboard::Quote:    out += "'"; break;
            case sf::Keyboard::Slash:    out += "/"; break;
            case sf::Keyboard::Backslash: out += "\\"; break;
            case sf::Keyboard::Tilde:    out += "~"; break;
            case sf::Keyboard::Equal:    out += "="; break;
            case sf::Keyboard::Hyphen:   out += "-"; break;
            case sf::Keyboard::Space:    out += "space"; break;
            case sf::Keyboard::Enter:    out += "enter"; break;
            case sf::Keyboard::Tab:      out += "tab"; break;
            case sf::Keyboard::Add:      out += "+"; break;
            case sf::Keyboard::Subtract: out += "-"; break;
            case sf::Keyboard::Multiply: out += "*"; break;
            case sf::Keyboard::Divide:   out += "/"; break;
            case sf::Keyboard::Left:     out += "left"; break;
            case sf::Keyboard::Right:    out += "right"; break;
            case sf::Keyboard::Up:       out += "up"; break;
            case sf::Keyboard::Down:     out += "down"; break;
            default: out += "unknown"; break;
            }
        }

        return Value(out);
    }
    return Value("event-empty");
}

Value sf_window_clear(const std::vector<Value>& n)
{
    if (n.size() != 3)
        throw std::runtime_error("sfWindowClear needs 3 arguments: r, g and b");
    if (n[0].valueType() != ValueType::Number)
        throw Ark::TypeError("sfWindowClear: r must be a Number");
    if (n[1].valueType() != ValueType::Number)
        throw Ark::TypeError("sfWindowClear: g must be a Number");
    if (n[2].valueType() != ValueType::Number)
        throw Ark::TypeError("sfWindowClear: b must be a Number");
    get_window().clear(sf::Color(static_cast<long>(n[0].number()), static_cast<long>(n[1].number()), static_cast<long>(n[2].number())));
    return Nil;
}

Value sf_draw(const std::vector<Value>& n)
{
    for (Value::Iterator it=n.begin(); it != n.end(); ++it)
    {
        if (it->valueType() != ValueType::String)
            throw Ark::TypeError("sfDraw: invalid argument");
        
        std::size_t i = it->string().find_first_of('-');
        std::string sub = it->string().substr(0, i);

        if (sub == "text")
            get_window().draw(get_texts()[n[0].string()]);
        else if (sub == "sprite")
            get_window().draw(get_sprites()[n[0].string()]);
        else if (sub == "event")
            throw Ark::TypeError("sfDraw: Can not draw event");
        else if (sub == "font")
            throw Ark::TypeError("sfDraw: Can not draw font");
        else
            throw Ark::TypeError("Object isn't a SFML object");
    }
    return Nil;
}

Value sf_window_display(const std::vector<Value>& n)
{
    get_window().display();
    return Nil;
}

Value sf_window_set_fps(const std::vector<Value>& n)
{
    if (n[0].valueType() != ValueType::Number)
        throw Ark::TypeError("sfWindowSetFPS: fps must be a Number");
    get_window().setFramerateLimit(static_cast<long>(n[0].number()));
    return Nil;
}

Value sf_load_sprite(const std::vector<Value>& n)
{
    if (n.size() != 1)
        throw std::runtime_error("sfLoadSprite: need 1 argument: path to sprite");
    if (n[0].valueType() != ValueType::String)
        throw Ark::TypeError("sfLoadSprite: need a String");
    
    std::string name = "sprite-" + n[0].string();
    
    get_textures()[name] = sf::Texture();
    if (!get_textures()[name].loadFromFile(n[0].string()))
        throw std::runtime_error("sfLoadSprite: Could not load sprite: " + n[0].string());
    
    get_sprites()[name] = sf::Sprite();
    get_sprites()[name].setTexture(get_textures()[name]);

    return Value(name);
}

Value sf_load_font(const std::vector<Value>& n)
{
    if (n.size() != 1)
        throw std::runtime_error("sfLoadFont: need 1 argument: path to font");
    if (n[0].valueType() != ValueType::String)
        throw Ark::TypeError("sfLoadFont: need a String");
    
    std::string name = "font-" + n[0].string();
    get_fonts()[name] = sf::Font();
    if (!get_fonts()[name].loadFromFile(n[0].string()))
        throw std::runtime_error("sfLoadFont: Could not load font: " + n[0].string());

    return Value(name);
}

Value sf_make_text(const std::vector<Value>& n)
{
    if (n.size() != 4)
        throw std::runtime_error("sfMakeText: need 4 arguments: font, text, size, color");
    if (n[0].valueType() != ValueType::String)
        throw Ark::TypeError("sfMakeText: invalid argument (font)");
    if (n[1].valueType() != ValueType::String)
        throw Ark::TypeError("sfMakeText: invalid argument (text)");
    if (n[2].valueType() != ValueType::Number)
        throw Ark::TypeError("sfMakeText: invalid argument (size)");
    if (n[3].valueType() != ValueType::List)
        throw Ark::TypeError("sfMakeText: invalid argument (color)");
    
    std::size_t i = n[0].string().find_first_of('-');
    std::string sub = n[0].string().substr(0, i);

    if (sub != "font")
        throw Ark::TypeError("sfMakeText: invalid font object");
    
    std::string name = "text-" + n[0].string() + n[1].string();
    get_texts()[name] = sf::Text();
    get_texts()[name].setFont(get_fonts()[n[0].string()]);
    get_texts()[name].setString(n[1].string());
    get_texts()[name].setCharacterSize(static_cast<long>(n[2].number()));
    get_texts()[name].setFillColor(sf::Color(
        static_cast<long>(n[3].const_list()[0].number()),
        static_cast<long>(n[3].const_list()[1].number()),
        static_cast<long>(n[3].const_list()[2].number())
    ));

    return Value(name);
}

Value sf_set_text(const std::vector<Value>& n)
{
    if (n.size() != 2)
        throw std::runtime_error("sfMakeText: need 2 arguments: text object, new value");
    if (n[0].valueType() != ValueType::String)
        throw Ark::TypeError("sfMakeText: invalid argument (text object)");
    if (n[1].valueType() != ValueType::String)
        throw Ark::TypeError("sfMakeText: invalid argument (new value)");
    
    std::size_t i = n[0].string().find_first_of('-');
    std::string sub = n[0].string().substr(0, i);

    if (sub == "text")
        get_texts()[n[0].string()].setString(n[1].string());
    else
        throw Ark::TypeError("Object isn't a text object");
    
    return Nil;
}

Value sf_setpos(const std::vector<Value>& n)
{
    if (n.size() != 3)
        throw std::runtime_error("sfSetPos: need 3 arguments: object, x, y");
    if (n[0].valueType() != ValueType::String)
        throw Ark::TypeError("sfSetPos: invalid argument (object)");
    if (n[1].valueType() != ValueType::Number)
        throw Ark::TypeError("sfSetPos: invalid argument (x)");
    if (n[2].valueType() != ValueType::Number)
        throw Ark::TypeError("sfSetPos: invalid argument (y)");
    
    std::size_t i = n[0].string().find_first_of('-');
    std::string sub = n[0].string().substr(0, i);

    if (sub == "text")
        get_texts()[n[0].string()].setPosition(static_cast<long>(n[1].number()), static_cast<long>(n[2].number()));
    else if (sub == "sprite")
        get_sprites()[n[0].string()].setPosition(static_cast<long>(n[1].number()), static_cast<long>(n[2].number()));
    else if (sub == "event")
        throw Ark::TypeError("sfSetPos: Can not set position of event");
    else if (sub == "font")
        throw Ark::TypeError("sfSetPos: Can not set position of font");
    else
        throw Ark::TypeError("Object isn't a SFML object");
    
    return Nil;
}

Value sf_width(const std::vector<Value>& n)
{
    if (n.size() != 1)
        throw std::runtime_error("sfWidth: need 1 argument: object");
    if (n[0].valueType() != ValueType::String)
        throw Ark::TypeError("sfWidth: invalid argument");
    
    std::size_t i = n[0].string().find_first_of('-');
    std::string sub = n[0].string().substr(0, i);

    if (sub == "text")
        return Value(static_cast<int>(get_texts()[n[0].string()].getGlobalBounds().width));
    else if (sub == "sprite")
        return Value(static_cast<int>(get_sprites()[n[0].string()].getGlobalBounds().width));
    else if (sub == "event")
        throw Ark::TypeError("sfWidth: Can not get width of event");
    else if (sub == "font")
        throw Ark::TypeError("sfWidth: Can not get width of font");
    throw Ark::TypeError("Object isn't a SFML object");
}

Value sf_height(const std::vector<Value>& n)
{
    if (n.size() != 1)
        throw std::runtime_error("sfHeight: need 1 argument: object");
    if (n[0].valueType() != ValueType::String)
        throw Ark::TypeError("sfHeight: invalid argument");
    
    std::size_t i = n[0].string().find_first_of('-');
    std::string sub = n[0].string().substr(0, i);

    if (sub == "text")
        return Value(static_cast<int>(get_texts()[n[0].string()].getGlobalBounds().height));
    else if (sub == "sprite")
        return Value(static_cast<int>(get_sprites()[n[0].string()].getGlobalBounds().height));
    else if (sub == "event")
        throw Ark::TypeError("sfHeight: Can not get height of event");
    else if (sub == "font")
        throw Ark::TypeError("sfHeight: Can not get height of font");
    throw Ark::TypeError("Object isn't a SFML object");
}

Value sf_event(const std::vector<Value>& n)
{
    std::string out = "event-";

    if (n.size() == 0)
        throw std::runtime_error("sfEvent need at least 1 argument");
    
    for (Value::Iterator it=n.begin(); it != n.end(); ++it)
    {
        if (it->valueType() != ValueType::String)
            throw Ark::TypeError("sfEvent: invalid argument");
        out += it->string();
    }

    return Value(out);
}

Value sf_window_close(const std::vector<Value>& n)
{
    get_window().close();
    return Nil;
}