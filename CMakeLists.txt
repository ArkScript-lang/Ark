cmake_minimum_required(VERSION 3.8)

project(Ark CXX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_CLANG ON)
endif()

# setting up compilations options

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg -g -no-pie")
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -s")
elseif (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/DWIN32 /D_WINDOWS /W3 /GR /EHa /MDd" CACHE STRING "Debug compilation options" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /DWIN32 /D_WINDOWS /W3 /GR /EHa /Ox /Ob2 /Oi /Ot /Oy /MD" CACHE STRING "Release compilation options" FORCE)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath,/usr/local/lib")
elseif (CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -v")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# configuring Constants.hpp

# VERSION
set(ARK_VERSION_MAJOR 3)
set(ARK_VERSION_MINOR 0)
set(ARK_VERSION_PATCH 5)

message(STATUS "ArkScript version ${ARK_VERSION_MAJOR}.${ARK_VERSION_MINOR}.${ARK_VERSION_PATCH}")

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

file(GLOB_RECURSE SOURCE_FILES
    ${Ark_SOURCE_DIR}/src/*.cpp
    ${Ark_SOURCE_DIR}/thirdparty/*.cpp
)
list(REMOVE_ITEM SOURCE_FILES "${Ark_SOURCE_DIR}/src/main.cpp")

add_library(ArkReactor STATIC ${SOURCE_FILES})

# including content of project

target_include_directories(ArkReactor
    PUBLIC
        ${Ark_SOURCE_DIR}/include
        ${Ark_SOURCE_DIR}/thirdparty
)

# linking libraries

if ((CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG) AND (UNIX OR LINUX))
    target_link_libraries(ArkReactor PUBLIC stdc++fs)
endif()

if (UNIX OR LINUX)
    target_link_libraries(ArkReactor PUBLIC ${CMAKE_DL_LIBS})
endif()

# setting up project properties

set_target_properties(
    ArkReactor
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

# building exe

if (ARK_BUILD_EXE)
    add_executable(Ark ${Ark_SOURCE_DIR}/src/main.cpp)
    target_include_directories(Ark PUBLIC
        ${Ark_SOURCE_DIR}/include
        ${Ark_SOURCE_DIR}/thirdparty
    )
    target_link_libraries(Ark PUBLIC ArkReactor)

    set_target_properties(
        Ark
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    # setting up installation directory

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

# submodules

if (ARK_BUILD_MODULES)
    add_subdirectory(${Ark_SOURCE_DIR}/modules)
endif()

if (ARK_BUILD_BENCHMARK)
    add_subdirectory(benchmarks)
endif()