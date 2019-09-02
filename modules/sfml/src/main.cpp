#include <Ark/Module.hpp>
#include <sf_module.hpp>

// module functions mapping
ARK_API_EXPORT Mapping_t getFunctionsMapping()
{
    Mapping_t map;
    map["sf-window-init"] = sf_window_init;
    map["sf-window-isOpen"] = sf_window_isopen;
    map["sf-poll-event"] = sf_poll_event;
    map["sf-window-clear"] = sf_window_clear;
    map["sf-draw"] = sf_draw;
    map["sf-window-display"] = sf_window_display;
    map["sf-window-setFPS"] = sf_window_set_fps;
    map["sf-load-sprite"] = sf_load_sprite;
    map["sf-load-font"] = sf_load_font;
    map["sf-make-text"] = sf_make_text;
    map["sf-setText"] = sf_set_text;
    map["sf-setPos"] = sf_setpos;
    map["sf-width"] = sf_width;
    map["sf-height"] = sf_height;
    map["sf-event"] = sf_event;
    map["sf-window-close"] = sf_window_close;

    return map;
}