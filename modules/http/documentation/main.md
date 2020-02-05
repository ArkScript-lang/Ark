# HTTP module

A module to play with HTTP requests and create web servers, using [cpp-httplib](https://github.com/yhirose/cpp-httplib) (MIT License).

## Functions

### httpCreateServer

Create a server.

Example:

```clojure
(let srv (httpCreateServer))
```

### httpServerGet

Create a route to answer GET requests.

The route and the content should both be of type String.

Example:

```clojure
{
    (let srv (httpCreateServer))
    (httpServerGet srv "/hi" "this is my fabulous content")
}
```

### httpServerStop

Stop a server.

Example:

```clojure
(httpServerStop srv)
```

### httpServerListen

Setup the server to listen forever on a given host (String) and port (Number). Should be called after having setup all the routes.

Example:

```clojure
{
    (let srv (httpCreateServer))

    (httpServerGet srv "/hi" "this is my fabulous content")
    # more routes...

    (httpServerListen srv "localhost" 1234)
}
```

### httpServerBindToAnyPort

Bind a socket to any available port.

Example:

```clojure
{
    (let srv (httpCreateServer))

    # binding to host 0.0.0.0, thus to multiple interfaces
    (httpServerBindToAnyPort srv "0.0.0.0")
}
```

### httpServerListenAfterBind

If you've done a `httpServerBindToAnyPort`, you shouldn't call `httpServerListen` but `httpServerListenAfterBind` since the server was already binded to an host and port.

Example:

```clojure
{
    (let srv (httpCreateServer))

    # binding to host 0.0.0.0, thus to multiple interfaces
    (httpServerBindToAnyPort srv "0.0.0.0")

    (httpServerListenAfterBind)
}
```