set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ark_SOURCE_DIR}/public")

# ArkScript lib needs to know if we are building an exe to enable/disable specific code for embedding
target_compile_definitions(ArkReactor PRIVATE ARK_BUILD_EXE=1)

add_executable(ArkEmscripten main.cpp)
target_link_libraries(ArkEmscripten PUBLIC ArkReactor)
target_compile_features(ArkEmscripten PRIVATE cxx_std_20)

if (ARK_JS_ONLY)
    message(STATUS "Setting compilation target to native JavaScript")
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    set_target_properties(ArkReactor PROPERTIES LINK_FLAGS "-s WASM=0 -s NO_DISABLE_EXCEPTION_CATCHING")
    set_target_properties(ArkEmscripten PROPERTIES LINK_FLAGS "-s WASM=0 -s NO_DISABLE_EXCEPTION_CATCHING -s EXPORTED_FUNCTIONS='[_main]' -s EXPORTED_RUNTIME_METHODS=['run'] -lembind")
else ()
    message(STATUS "Setting compilation target to WASM")
    set(CMAKE_EXECUTABLE_SUFFIX ".wasm.js")
    set_target_properties(ArkReactor PROPERTIES LINK_FLAGS "-s WASM=1 -s NO_DISABLE_EXCEPTION_CATCHING")
    set_target_properties(ArkEmscripten PROPERTIES LINK_FLAGS "-s WASM=1 -s NO_DISABLE_EXCEPTION_CATCHING -s EXPORTED_FUNCTIONS='[_main]' -s EXPORTED_RUNTIME_METHODS=['run'] -lembind")
endif ()
