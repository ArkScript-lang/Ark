#include <Ark/Module.hpp>
#include <sf_module.hpp>

// module functions mapping
ARK_API_EXPORT Mapping_t getFunctionsMapping()
{
    Mapping_t map;
    map["sfWindowInit"] = sf_window_init;
    map["sfWindowIsOpen"] = sf_window_isopen;
    map["sfPollEvent"] = sf_poll_event;
    map["sfWindowClear"] = sf_window_clear;
    map["sfDraw"] = sf_draw;
    map["sfWindowDisplay"] = sf_window_display;
    map["sfWindowSetFPS"] = sf_window_set_fps;
    map["sfLoadSprite"] = sf_load_sprite;
    map["sfLoadFont"] = sf_load_font;
    map["sfMakeText"] = sf_make_text;
    map["sfSetText"] = sf_set_text;
    map["sfSetPos"] = sf_setpos;
    map["sfWidth"] = sf_width;
    map["sfHeight"] = sf_height;
    map["sfEvent"] = sf_event;
    map["sfWindowClose"] = sf_window_close;

    return map;
}