cmake_minimum_required(VERSION 3.8)

project(Ark CXX)

# setting up compilations options

if (NOT BUILD_MODULES)
    message(STATUS "To build the modules, use -DBUILD_MODULES=\"true\"")
    message(STATUS "Not building the modules")

    set(BUILD_MODULES "false" CACHE STRING "Build the modules" FORCE)
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    if (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g -no-pie")
    elseif (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MTd")
    endif()
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    if (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "-DNDEBUG -O3 -s")
    elseif (MSVC)
        set(CMAKE_CXX_FLAGS "/Ox /Ob2 /Oi /Ot /Oy /MT")
    endif()
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath,/usr/local/lib")
endif()

if (BUILD_MODULES MATCHES "true")
    add_subdirectory(${Ark_SOURCE_DIR}/modules)
endif()

# configuring Constants.hpp

# VERSION
set(ARK_VERSION_MAJOR 3)
set(ARK_VERSION_MINOR 0)
set(ARK_VERSION_PATCH 1)

# COMPILATION RELATED
set(ARK_COMPILATION_OPTIONS ${CMAKE_CXX_FLAGS})
set(ARK_COMPILER ${CMAKE_CXX_COMPILER_ID})
set(ARK_SUBMODULES_ROOT ${Ark_SOURCE_DIR}/submodules)

# STANDARD LIBRARY PATH
if (UNIX OR LINUX)
    set(ARK_STD ${CMAKE_INSTALL_PREFIX}/share/.Ark/lib)
elseif (WIN32)
    if (MSVC)
        set(ARK_STD ${CMAKE_INSTALL_PREFIX}/lib)
    endif()
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
list(REMOVE_ITEM SOURCE_FILES "${Ark_SOURCE_DIR}/src/main.cpp")

add_library(ArkReactor ${SOURCE_FILES})
set_property(TARGET ArkReactor PROPERTY POSITION_INDEPENDENT_CODE ON)

if (CMAKE_COMPILER_IS_GNUCXX AND (UNIX OR LINUX))
    target_link_libraries(ArkReactor PUBLIC stdc++fs)
endif()

if (UNIX OR LINUX)
    target_link_libraries(ArkReactor PUBLIC ${CMAKE_DL_LIBS})
endif()

add_executable(Ark ${Ark_SOURCE_DIR}/src/main.cpp)
target_link_libraries(Ark PUBLIC ArkReactor)

set_target_properties(
    Ark
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

set_target_properties(
    ArkReactor
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

add_subdirectory(benchmarks)

if (UNIX OR LINUX)
    install(TARGETS Ark DESTINATION bin)
    install(DIRECTORY ${Ark_SOURCE_DIR}/lib/ DESTINATION share/.Ark/lib)
elseif (WIN32)
    if (MSVC)
        install(TARGETS Ark DESTINATION bin)
        install(DIRECTORY ${Ark_SOURCE_DIR}/lib DESTINATION lib)
    endif()
endif()