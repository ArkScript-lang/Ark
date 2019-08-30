cmake_minimum_required(VERSION 3.8)

project(Ark CXX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_CLANG ON)
endif()

# setting up compilations options

if (CMAKE_BUILD_TYPE MATCHES Debug)
    if (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g -no-pie")
    elseif (MSVC)
        set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /GR /EHa /MTd")
    endif()
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    if (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "-DNDEBUG -O3 -s")
    elseif (MSVC)
        set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /GR /EHa /Ox /Ob2 /Oi /Ot /Oy /MT")
    endif()
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath,/usr/local/lib")
elseif (CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -v")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

if (ARK_BUILD_MODULES)
    add_subdirectory(${Ark_SOURCE_DIR}/modules)
endif()

# configuring Constants.hpp

# VERSION
set(ARK_VERSION_MAJOR 3)
set(ARK_VERSION_MINOR 0)
set(ARK_VERSION_PATCH 3)

# COMPILATION RELATED
set(ARK_COMPILATION_OPTIONS ${CMAKE_CXX_FLAGS})
set(ARK_COMPILER ${CMAKE_CXX_COMPILER_ID})
set(ARK_SUBMODULES_ROOT ${Ark_SOURCE_DIR}/submodules)
set(ARK_ENABLE_SYSTEM 1)  # enable use of (system "command here")

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

if ((CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG) AND (UNIX OR LINUX))
    target_link_libraries(ArkReactor PUBLIC stdc++fs)
endif()

if (UNIX OR LINUX)
    target_link_libraries(ArkReactor PUBLIC ${CMAKE_DL_LIBS})
endif()

if (ARK_BUILD_EXE)
    add_executable(Ark ${Ark_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(Ark PUBLIC ArkReactor)

    set_target_properties(
        Ark
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    if (UNIX OR LINUX)
        install(TARGETS Ark DESTINATION bin)
        install(DIRECTORY ${Ark_SOURCE_DIR}/lib/ DESTINATION share/.Ark/lib)
    elseif (WIN32)
        if (MSVC)
            install(TARGETS Ark DESTINATION bin)
            install(DIRECTORY ${Ark_SOURCE_DIR}/lib/ DESTINATION lib)
        endif()
    endif()
endif()

set_target_properties(
    ArkReactor
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

if (ARK_BUILD_BENCHMARK)
    add_subdirectory(benchmarks)
endif()