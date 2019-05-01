cmake_minimum_required(VERSION 3.8)

project(Ark CXX)

if (NOT DEBUG)
    message(STATUS "To enable debug mode, use -DDEBUG=true")
    message(STATUS "Falling back to release")
    
    set(DEBUG "false" CACHE STRING "Enables debug mode" FORCE)
endif()

if (NOT TESTCPP)
    message(STATUS "To use test.cpp instead of main.cpp, use -DTESTCPP=true")
    message(STATUS "Using main.cpp")
    set(TESTCPP "false" CACHE STRING "Use test.cpp instead of main.cpp" FORCE)
endif()

set(CMAKE_MODULE_PATH ${Ark_SOURCE_DIR}/cmake)
find_package(GMP REQUIRED)

include_directories(
    ${Ark_SOURCE_DIR}/include
    ${Ark_SOURCE_DIR}/thirdparty

    ${GMP_INCLUDE_DIR}
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
target_link_libraries(Ark PRIVATE ${GMP_LIBRARIES})
target_link_libraries(Ark PRIVATE stdc++fs)

set_target_properties(
    Ark
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

if (DEBUG MATCHES "true")
    if (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "-pg -g -no-pie")
    endif()
endif()