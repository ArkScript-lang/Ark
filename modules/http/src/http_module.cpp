#include <Ark/Module.hpp>
#include <httplib.hpp>

using namespace httplib;

enum class Type : unsigned
{
    Server = 0,
    Client = 1
};

int& get_logger_level()
{
    static int i = 0;
    return i;
}

bool& get_error_handler()
{
    static bool b = false;
    return b;
}

Server& create_server()
{
    static Server srv;

    if (get_logger_level() > 0)
        srv.set_logger([](const auto& req, const auto& res) {
            std::cout << "got request\n";
            std::cout << "method " << req.method << ", path " << req.path << ", body " << req.body << "\n";
            std::cout << "status " << res.status << "\n";
            std::cout << "==================\n\n";
        });
    
    if (get_error_handler())
        srv.set_error_handler([](const auto& req, const auto& res) {
            std::cout << "ERROR???\n";
            std::cout << "status " << res.status << "\n";
            std::cout << "==================\n\n";
        });
    
    return srv;
}

Value http_create_server(const std::vector<Value>& n)
{
    Value server = Ark::Value(Ark::UserType(static_cast<unsigned>(Type::Server), &create_server()));
    server.usertype_ref().setOStream([](std::ostream& os, const UserType& A) -> std::ostream& {
        os << "httpServer<0x" << A.data() << ">";
        return os;
    });
    return server;
}

Value http_server_get(const std::vector<Value>& n)
{
    if (n.size() < 3 || n.size() > 4)
        throw std::runtime_error("httpServerGet needs 3 to 4 arguments: server, route, content, [type=text/plain]");
    if (n[0].valueType() != ValueType::User || n[0].usertype().type_id() != static_cast<unsigned>(Type::Server))
        throw Ark::TypeError("httpServerGet: server must be an httpServer");
    if (n[1].valueType() != ValueType::String)
        throw Ark::TypeError("httpServerGet: route must be a String");
    if (n[2].valueType() != ValueType::String)
        throw Ark::TypeError("httpServerGet: content must be a String");

    std::string type = "text/plain";
    if (n.size() == 4)
    {
        if (n[3].valueType() != ValueType::String)
            throw Ark::TypeError("httpServGet: type must be a String");
        else
            type = n[3].string();
    }

    std::string content = n[2].string();

    Server *srv = static_cast<Server*>(n[0].usertype().data());
    srv->Get(n[1].string().c_str(), [content, type](const Request& req, Response& res) {
        // TODO allow use of req.matches
        // TODO allow use of external functions (eg, httpServerStop when going to /stop)
        res.set_content(content, type.c_str());
    });

    return Nil;
}

Value http_server_stop(const std::vector<Value>& n)
{
    if (n.size() != 1)
        throw std::runtime_error("httpServerStop: needs a single argument: httpServer");
    if (n[0].valueType() != ValueType::User || n[0].usertype().type_id() != static_cast<unsigned>(Type::Server))
        throw Ark::TypeError("httpServerGet: server must be an httpServer");
    
    static_cast<Server*>(n[0].usertype().data())->stop();
    
    return Nil;
}

Value http_server_listen(const std::vector<Value>& n)
{
    if (n.size() != 3)
        throw std::runtime_error("httpServerListen: needs 3 arguments: httpServer, host, port");
    if (n[0].valueType() != ValueType::User || n[0].usertype().type_id() != static_cast<unsigned>(Type::Server))
        throw Ark::TypeError("httpServerListen: server must be an httpServer");
    if (n[1].valueType() != ValueType::String)
        throw Ark::TypeError("httpServerListen: host must be a String");
    if (n[2].valueType() != ValueType::Number)
        throw Ark::TypeError("httpServerListen: port must be a Number");

    static_cast<Server*>(n[0].usertype().data())->listen(n[1].string().c_str(), static_cast<int>(n[2].number()));
    
    return Nil;
}

Value http_server_bind_to_any_port(const std::vector<Value>& n)
{
    if (n.size() != 2)
        throw std::runtime_error("httpServerBindToAnyPort: needs 2 arguments: httpServer, host");
    if (n[0].valueType() != ValueType::User || n[0].usertype().type_id() != static_cast<unsigned>(Type::Server))
        throw Ark::TypeError("httpServerBindToAnyPort: server must be an httpServer");
    if (n[1].valueType() != ValueType::String)
        throw Ark::TypeError("httpServerBindToAnyPort: host must be a String");
    
    return Value(static_cast<Server*>(n[0].usertype().data())->bind_to_any_port(n[1].string().c_str()));
}

Value http_server_listen_after_bind(const std::vector<Value>& n)
{
    if (n.size() != 1)
        throw std::runtime_error("httpServerListenAfterBind: needs a single argument: httpServer");
    if (n[0].valueType() != ValueType::User || n[0].usertype().type_id() != static_cast<unsigned>(Type::Server))
        throw Ark::TypeError("httpServerListenAfterBind: server must be an httpServer");
    
    static_cast<Server*>(n[0].usertype().data())->listen_after_bind();

    return Nil;
}

Value http_server_set_mount_point(const std::vector<Value>& n)
{
    if (n.size() != 3)
        throw std::runtime_error("httpServerSetMountPoint: needs 3 arguments: httpServer, folder, destination");
    if (n[0].valueType() != ValueType::User || n[0].usertype().type_id() != static_cast<unsigned>(Type::Server))
        throw Ark::TypeError("httpServerSetMountPoint: server must be an httpServer");
    if (n[1].valueType() != ValueType::String)
        throw Ark::TypeError("httpServerSetMountPoint: folder must be a String");
    if (n[2].valueType() != ValueType::String)
        throw Ark::TypeError("httpServerSetMountPoint: destination must be a String");
    
    auto ret = static_cast<Server*>(n[0].usertype().data())->set_mount_point(n[1].string().c_str(), n[2].string().c_str());
    if (!ret)
        return False;  // directory doesn't exist
    return True;
}

Value http_server_remove_mount_point(const std::vector<Value>& n)
{
    if (n.size() != 2)
        throw std::runtime_error("httpServerRmMountPoint: needs 2 arguments: httpServer, folder");
    if (n[0].valueType() != ValueType::User || n[0].usertype().type_id() != static_cast<unsigned>(Type::Server))
        throw Ark::TypeError("httpServerRmMountPoint: server must be an httpServer");
    if (n[1].valueType() != ValueType::String)
        throw Ark::TypeError("httpServerRmMountPoint: folder must be a String");
    
    auto ret = static_cast<Server*>(n[0].usertype().data())->remove_mount_point(n[1].string().c_str());
    if (!ret)
        return False;  // directory doesn't exist
    return True;
}

Value http_server_set_fext_mimetype(const std::vector<Value>& n)
{
    if (n.size() != 3)
        throw std::runtime_error("httpServerSetFileExtAndMimetypeMapping: needs 3 arguments: httpServer, ext, mimetype");
    if (n[0].valueType() != ValueType::User || n[0].usertype().type_id() != static_cast<unsigned>(Type::Server))
        throw Ark::TypeError("httpServerSetFileExtAndMimetypeMapping: server must be an httpServer");
    if (n[1].valueType() != ValueType::String)
        throw Ark::TypeError("httpServerSetFileExtAndMimetypeMapping: ext must be a String");
    if (n[2].valueType() != ValueType::String)
        throw Ark::TypeError("httpServerSetFileExtAndMimetypeMapping: mimetype must be a String");
    
    static_cast<Server*>(n[0].usertype().data())->set_file_extension_and_mimetype_mapping(
        n[1].string().c_str(), n[2].string().c_str()
    );
    return Nil;
}

Value http_server_enable_logger(const std::vector<Value>& n)
{
    if (n.size() > 1)
        throw std::runtime_error("httpServerEnableLogger: needs 0 or 1 argument: [level=1]");
    
    if (n.size() == 1 && n[0].valueType() != ValueType::String)
        throw Ark::TypeError("httpServerEnableLogger: level must be a Number");

    int& level = get_logger_level();
    if (n.size() == 1)
        level = static_cast<int>(n[0].number());
    else
        level = 1;

    return Nil;
}