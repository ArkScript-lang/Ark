#include <SFML/Graphics.hpp>
#include <module_base.hpp>

bool has_window = false;
sf::RenderWindow window;
sf::Event event;
std::vector<sf::Texture> textures;
std::unordered_map<std::string, sf::Sprite> sprites;
std::unordered_map<std::string, sf::Font> fonts;
std::unordered_map<std::string, sf::Text> texts;

// module functions
Value sf_window_init(const std::vector<Value>& n)
{
    if (n.size() != 3)
        throw std::runtime_error("sf-window-init needs 3 arguments: width, height and title");
    if (!n[0].isNumber())
        throw Ark::TypeError("sf-window-init: width must be a Number");
    if (!n[1].isNumber())
        throw Ark::TypeError("sf-window-init: height must be a Number");
    if (!n[2].isString())
        throw Ark::TypeError("sf-window-init: title must be a String");
    
    if (!has_window)
    {
        window.create(sf::VideoMode(n[0].number().toLong(), n[1].number().toLong()), n[2].string());
        has_window = true;
    }
    else
        throw std::runtime_error("sf-window-init: can't call the function twice");

    return nil;
}

Value sf_window_isopen(const std::vector<Value>& n)
{
    return window.isOpen() ? trueSym : falseSym;
}

Value sf_poll_event(const std::vector<Value>& n)
{
    if (window.pollEvent(event))
    {
        std::string out = "event-";

        if (event.type == sf::Event::Closed)
            out += "quit";
        else if (event.type == sf::Event::KeyReleased)
            out += "keyup";
        else if (event.type == sf::Event::KeyPressed)
            out += "keydown";

        return Value(out);
    }
    return Value("event-empty");
}

Value sf_window_clear(const std::vector<Value>& n)
{
    if (n.size() != 3)
        throw std::runtime_error("sf-window-clear needs 3 arguments: r, g and b");
    if (!n[0].isNumber())
        throw Ark::TypeError("sf-window-clear: r must be a Number");
    if (!n[1].isNumber())
        throw Ark::TypeError("sf-window-clear: g must be a Number");
    if (!n[2].isNumber())
        throw Ark::TypeError("sf-window-clear: b must be a Number");
    window.clear(sf::Color(n[0].number().toLong(), n[1].number().toLong(), n[2].number().toLong()));
    return nil;
}

Value sf_draw(const std::vector<Value>& n)
{
    for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
    {
        if (!it->isString())
            throw Ark::TypeError("sf-draw: invalid argument");
        
        std::size_t i = it->string().find_first_of('-');
        std::string sub = it->string().substr(0, i);

        if (sub == "text")
            window.draw(texts[n[0].string()]);
        else if (sub == "sprite")
            window.draw(sprites[n[0].string()]);
        else if (sub == "event")
            throw Ark::TypeError("sf-draw: Can not draw event");
        else if (sub == "font")
            throw Ark::TypeError("sf-draw: Can not draw font");
        else
            throw Ark::TypeError("Object isn't a SFML object");
    }
    return nil;
}

Value sf_window_display(const std::vector<Value>& n)
{
    window.display();
    return nil;
}

Value sf_window_set_fps(const std::vector<Value>& n)
{
    if (!n[0].isNumber())
        throw Ark::TypeError("sf-window-setFPS: fps must be a Number");
    window.setFramerateLimit(n[0].number().toLong());
    return nil;
}

Value sf_load_sprite(const std::vector<Value>& n)
{
    if (n.size() != 1)
        throw std::runtime_error("sf-load-sprite: need 1 argument: path to sprite");
    if (!n[0].isString())
        throw Ark::TypeError("sf-load-sprite: need a String");
    
    textures.emplace_back();
    if (!textures.back().loadFromFile(n[0].string()))
        throw std::runtime_error("sf-load-sprite: Could not load sprite: " + n[0].string());
    std::string name = "sprite-" + n[0].string();
    sprites[name] = sf::Sprite();
    sprites[name].setTexture(textures.back());

    return Value(name);
}

Value sf_load_font(const std::vector<Value>& n)
{
    if (n.size() != 1)
        throw std::runtime_error("sf-load-font: need 1 argument: path to font");
    if (!n[0].isString())
        throw Ark::TypeError("sf-load-font: need a String");
    
    std::string name = "font-" + n[0].string();
    fonts[name] = sf::Font();
    if (!fonts[name].loadFromFile(n[0].string()))
        throw std::runtime_error("sf-load-font: Could not load font: " + n[0].string());

    return Value(name);
}

Value sf_make_text(const std::vector<Value>& n)
{
    if (n.size() != 4)
        throw std::runtime_error("sf-make-text: need 4 arguments: font, text, size, color");
    if (!n[0].isString())
        throw Ark::TypeError("sf-make-text: invalid argument (font)");
    if (!n[1].isString())
        throw Ark::TypeError("sf-make-text: invalid argument (text)");
    if (!n[2].isNumber())
        throw Ark::TypeError("sf-make-text: invalid argument (size)");
    if (!n[3].isList())
        throw Ark::TypeError("sf-make-text: invalid argument (color)");
    
    std::size_t i = n[0].string().find_first_of('-');
    std::string sub = n[0].string().substr(0, i);

    if (sub != "font")
        throw Ark::TypeError("sf-make-text: invalid font object");
    
    std::string name = "text-" + n[0].string() + n[1].string();
    texts[name] = sf::Text();
    texts[name].setFont(fonts[n[0].string()]);
    texts[name].setString(n[1].string());
    texts[name].setCharacterSize(n[2].number().toLong());
    texts[name].setFillColor(sf::Color(
        n[3].const_list()[0].number().toLong(),
        n[3].const_list()[1].number().toLong(),
        n[3].const_list()[2].number().toLong()
    ));

    return Value(name);
}

Value sf_setpos(const std::vector<Value>& n)
{
    if (n.size() != 3)
        throw std::runtime_error("sf-setPos: need 3 arguments: object, x, y");
    if (!n[0].isString())
        throw Ark::TypeError("sf-setPos: invalid argument (object)");
    if (!n[1].isNumber())
        throw Ark::TypeError("sf-setPos: invalid argument (x)");
    if (!n[2].isNumber())
        throw Ark::TypeError("sf-setPos: invalid argument (y)");
    
    std::size_t i = n[0].string().find_first_of('-');
    std::string sub = n[0].string().substr(0, i);

    if (sub == "text")
        texts[n[0].string()].setPosition(n[1].number().toLong(), n[2].number().toLong());
    else if (sub == "sprite")
        sprites[n[0].string()].setPosition(n[1].number().toLong(), n[2].number().toLong());
    else if (sub == "event")
        throw Ark::TypeError("sf-setPos: Can not set position of event");
    else if (sub == "font")
        throw Ark::TypeError("sf-setPos: Can not set position of font");
    else
        throw Ark::TypeError("Object isn't a SFML object");
    
    return nil;
}

Value sf_width(const std::vector<Value>& n)
{
    if (n.size() != 1)
        throw std::runtime_error("sf-width: need 1 argument: object");
    if (!n[0].isString())
        throw Ark::TypeError("sf-width: invalid argument");
    
    std::size_t i = n[0].string().find_first_of('-');
    std::string sub = n[0].string().substr(0, i);

    if (sub == "text")
        return Value(static_cast<int>(texts[n[0].string()].getGlobalBounds().width));
    else if (sub == "sprite")
        return Value(static_cast<int>(sprites[n[0].string()].getGlobalBounds().width));
    else if (sub == "event")
        throw Ark::TypeError("sf-width: Can not get width of event");
    else if (sub == "font")
        throw Ark::TypeError("sf-width: Can not get width of font");
    throw Ark::TypeError("Object isn't a SFML object");
}

Value sf_height(const std::vector<Value>& n)
{
    if (n.size() != 1)
        throw std::runtime_error("sf-height: need 1 argument: object");
    if (!n[0].isString())
        throw Ark::TypeError("sf-height: invalid argument");
    
    std::size_t i = n[0].string().find_first_of('-');
    std::string sub = n[0].string().substr(0, i);

    if (sub == "text")
        return Value(static_cast<int>(texts[n[0].string()].getGlobalBounds().height));
    else if (sub == "sprite")
        return Value(static_cast<int>(sprites[n[0].string()].getGlobalBounds().height));
    else if (sub == "event")
        throw Ark::TypeError("sf-height: Can not get height of event");
    else if (sub == "font")
        throw Ark::TypeError("sf-height: Can not get height of font");
    throw Ark::TypeError("Object isn't a SFML object");
}

Value sf_event(const std::vector<Value>& n)
{
    std::string out = "event-";

    if (n.size() == 0)
        throw std::runtime_error("sf-event need at least 1 argument");
    
    for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
    {
        if (!it->isString())
            throw Ark::TypeError("sf-event: invalid argument");
        out += it->string();
    }

    return Value(out);
}

Value sf_window_close(const std::vector<Value>& n)
{
    window.close();
    return nil;
}