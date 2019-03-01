cmake_minimum_required(VERSION 3.8)

project(Ark CXX)

if (NOT TESTCPP)
    message(STATUS "To use test.cpp instead of main.cpp, use -DTESTCPP=true")
    message(STATUS "Using main.cpp")
    set(TESTCPP "false" CACHE STRING "Use test.cpp instead of main.cpp" FORCE)
endif()

include_directories(
    ${Ark_SOURCE_DIR}/include
    ${Ark_SOURCE_DIR}/thirdparty
)

file(GLOB_RECURSE SOURCE_FILES
    ${Ark_SOURCE_DIR}/src/*.cpp
    ${Ark_SOURCE_DIR}/thirdparty/*.cpp
)

if (TESTCPP MATCHES "true")
    list(REMOVE_ITEM SOURCE_FILES "${Ark_SOURCE_DIR}/src/main.cpp")
else()
    list(REMOVE_ITEM SOURCE_FILES "${Ark_SOURCE_DIR}/src/test.cpp")
endif()

add_executable(Ark ${SOURCE_FILES})

set_target_properties(
    Ark
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
