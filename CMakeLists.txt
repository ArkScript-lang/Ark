cmake_minimum_required(VERSION 3.8)

project(Ark CXX)

# setting up compilations options

if (NOT DEBUG)
    message(STATUS "To enable debug mode, use -DDEBUG=\"true\"")
    message(STATUS "Falling back to release")
    
    set(DEBUG "false" CACHE STRING "Enables debug mode" FORCE)
endif()

if (NOT BUILD_MODULES)
    message(STATUS "To build the modules, use -DBUILD_MODULES=\"true\"")
    message(STATUS "Not building the modules")

    set(BUILD_MODULES "false" CACHE STRING "Build the modules" FORCE)
endif()

if (NOT TESTCPP)
    message(STATUS "To use test.cpp instead of main.cpp, use -DTESTCPP=true")
    message(STATUS "Using main.cpp")
    set(TESTCPP "false" CACHE STRING "Use test.cpp instead of main.cpp" FORCE)
endif()

if (DEBUG MATCHES "true")
    if (CMAKE_COMPILER_IS_GNUCXX)
        message(STATUS "Debug mode ON")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g -no-pie")
    endif()
else()
    # release mode
    message(STATUS "Release mode ON")
    if (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "-DNDEBUG -O4 -s")
    elseif(MSVC)
        set(CMAKE_CXX_FLAGS "/Ox /Ob2 /Oi /Ot /Oy")
    endif()
endif()

# configuring Constants.hpp

set(ARK_VERSION_MAJOR 2)
set(ARK_VERSION_MINOR 2)
set(ARK_VERSION_PATCH 0)
set(ARK_STD ${CMAKE_INSTALL_PREFIX}/share/.Ark/lib)
set(ARK_COMPILATION_OPTIONS ${CMAKE_CXX_FLAGS})
set(ARK_COMPILER ${CMAKE_CXX_COMPILER_ID})

if (BUILD_MODULES MATCHES "true")
    add_subdirectory(${Ark_SOURCE_DIR}/modules)
endif()

configure_file(
    ${Ark_SOURCE_DIR}/include/Ark/Constants.hpp.in
    ${Ark_SOURCE_DIR}/include/Ark/Constants.hpp
)

# including content of project

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

if (CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(Ark PUBLIC stdc++fs)
endif()
if (UNIX OR LINUX)
    target_link_libraries(Ark PUBLIC ${CMAKE_DL_LIBS})
endif()

set_target_properties(
    Ark
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath,/usr/local/lib")
endif()

install(TARGETS Ark DESTINATION bin)
install(DIRECTORY ${Ark_SOURCE_DIR}/lib/ DESTINATION share/.Ark/lib)