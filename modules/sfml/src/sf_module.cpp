#include <SFML/Graphics.hpp>
#include <module_base.hpp>

bool has_window = false;
sf::RenderWindow window;

// module functions
Value sf_window_init(const std::vector<Value>& n)
{
    if (n.size() != 3)
        throw std::runtime_error("sf-window-init needs 3 arguments: width, height and title");
    if (!n[0].isNumber())
        throw std::runtime_error("sf-window-init: width must be a Number");
    if (!n[1].isNumber())
        throw std::runtime_error("sf-window-init: height must be a Number");
    if (!n[2].isString())
        throw std::runtime_error("sf-window-init: title must be a String");
    
    if (!has_window)
    {
        window.create(sf::VideoMode(n[0].number().toLong(), n[1].number().toLong()), n[2].string());
        has_window = true;
    }
    else
        throw std::runtime_error("sf-window-init: can't call the function twice");

    return nil;
}