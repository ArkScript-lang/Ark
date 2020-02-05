#include <Ark/Module.hpp>
#include <http_module.hpp>

// module functions mapping
ARK_API_EXPORT Mapping_t getFunctionsMapping()
{
    Mapping_t map;
    map["httpCreateServer"] = http_create_server;
    map["httpServerGet"] = http_server_get;
    map["httpServerStop"] = http_server_stop;
    map["httpServerListen"] = http_server_listen;
    map["httpServerBindToAnyPort"] = http_server_bind_to_any_port;
    map["httpServerListenAfterBind"] = http_server_listen_after_bind;

    return map;
}