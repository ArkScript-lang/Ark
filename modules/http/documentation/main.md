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

### httpServerSetMountPoint

Mount a given directory to a specific location.

You can mount a directory to multiple locations, thus creating a search order.

Returns a boolean: true if it worked, false if the base directory doesn't exist.

Example:

```clojure
{
    (let srv (httpCreateServer))

    # mount / to ./www
    (httpServerSetMountPoint srv "/" "./www")

    # mount /public to ./www1 and ./www2 directories
    (httpServerSetMountPoint srv "/public" "/www1")  # 1st order to search
    (httpServerSetMountPoint srv "/public" "/www2")  # 2nd order to search
}
```

### httpServerRmMountPoint

Remove a mount point. Returns false if the mount point can't be found, true otherwise.

Example:

```clojure
{
    (let srv (httpCreateServer))

    # mount / to ./www
    (httpServerSetMountPoint srv "/" "./www")

    # mount /public to ./www1 and ./www2 directories
    (httpServerSetMountPoint srv "/public" "/www1")  # 1st order to search
    (httpServerSetMountPoint srv "/public" "/www2")  # 2nd order to search

    # remove mount /
    (httpServerRmMountPoint srv "/")
}
```

### httpServerSetFileExtAndMimetypeMapping

Map a file extension to a mimetype.

Built-in mappings:

Extension | MIME Type
--------- | ---------
txt | text/plain
html, htm | text/html
css | text/css
jpeg, jpg | image/jpg
png | image/png
gif | image/gif
svg | image/svg+xml
ico | image/x-icon
json | application/json
pdf | application/pdf
js | application/javascript
wasm | application/wasm
xml | application/xml
xhtml | application/xhtml+xml

Example:

```clojure
(httpServerSetFileExtAndMimetypeMapping srv "cc" "text/x-c")
```

### httpServerEnableLogger

Set the logging level, by default 0.

TODO document the logging level

Example:

```clojure
{
    # set logger level to 1
    (httpServerEnableLogger)

    # select a given logger level, here 3
    (httpServerEnableLogger 3)
}
```