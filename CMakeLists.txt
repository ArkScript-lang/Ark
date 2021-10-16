cmake_minimum_required(VERSION 3.8)

project(ark CXX)

# VERSION
set(ARK_VERSION_MAJOR 3)
set(ARK_VERSION_MINOR 1)
set(ARK_VERSION_PATCH 1)

# Uses GNU Install directory variables
include(GNUInstallDirs)

# setting up compilations options

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_CLANG ON)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# files needed for the library ArkReactor
file(GLOB_RECURSE SOURCE_FILES
    ${ark_SOURCE_DIR}/src/arkreactor/*.cpp)

add_library(ArkReactor SHARED ${SOURCE_FILES})

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
    target_compile_options(ArkReactor
        PUBLIC
        -Wall -Wextra -pedantic -Wstrict-aliasing)
elseif (MSVC)
    target_compile_options(ArkReactor
        PUBLIC
        /W4
        /MP4      # build multiple source files concurrently
        /EHa      # set exception model to standard C++ stack unwinding
        /wd4267   # disable warning about data loss (size_t -> int)
        /wd4244   # disable warning about data loss (size_t -> char)
        /wd4505)  # disable warning about unused static function was deleted
endif()

# options specific to clang
if (CMAKE_COMPILER_IS_CLANG)
    target_compile_options(ArkReactor PUBLIC -Wunreachable-code -Wdeprecated -stdlib=libc++)
    target_link_options(ArkReactor PUBLIC -stdlib=libc++ -lc++abi)
endif()

# Link libraries

add_subdirectory("${ark_SOURCE_DIR}/lib/termcolor" EXCLUDE_FROM_ALL)
add_subdirectory("${ark_SOURCE_DIR}/lib/String/" EXCLUDE_FROM_ALL)

target_include_directories(ArkReactor
    PUBLIC
    "${ark_SOURCE_DIR}/lib/utf8_decoder/"
    "${ark_SOURCE_DIR}/lib/picosha2/"
    "${ark_SOURCE_DIR}/lib/String/include/")

target_link_libraries(ArkReactor PUBLIC termcolor ArkScriptString)

if (UNIX OR LINUX)
    if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
        target_link_libraries(ArkReactor PUBLIC stdc++fs)
    endif()
    find_package(Threads)
    target_link_libraries(ArkReactor PRIVATE ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

# coz-profiler
if (ARK_PROFILER)
    message(STATUS "Coz Profiler is enabled")

    find_package(coz-profiler REQUIRED PATHS ${ark_SOURCE_DIR}/lib/coz/)
    target_include_directories(ArkReactor PUBLIC ${COZ_INCLUDE_DIR})
    target_link_libraries(ArkReactor PUBLIC ${COZ_LIBRARY})
else()
    message(STATUS "Coz Profiler is NOT enabled")
endif()

# configuring Constants.hpp
message(STATUS "ArkScript version ${ARK_VERSION_MAJOR}.${ARK_VERSION_MINOR}.${ARK_VERSION_PATCH}")
configure_file(
    ${ark_SOURCE_DIR}/include/Ark/Constants.hpp.in
    ${ark_SOURCE_DIR}/include/Ark/Constants.hpp)

# including content of project
target_include_directories(ArkReactor
    PUBLIC
    ${ark_SOURCE_DIR}/include)

# setting up project properties
set_target_properties(
    ArkReactor
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

# Installation rules

# Installs the dynamic library file.
install(TARGETS ArkReactor
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Install header files
install(DIRECTORY ${ark_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install the standard library
if(NOT ARK_NO_STD)
    install(DIRECTORY ${ark_SOURCE_DIR}/lib/std/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/Ark/std
        FILES_MATCHING PATTERN "*.ark"
        PATTERN "std/tests" EXCLUDE
        PATTERN "std/.github" EXCLUDE)
endif()

# COMPILATION RELATED

target_compile_definitions(ArkReactor PRIVATE ARK_EXPORT)

option(ARK_BUILD_EXE        "Build a standalone arkscript executable" Off)
option(ARK_ENABLE_SYSTEM    "Enable sys:exec" On)  # enable use of (sys:exec "command here")
option(ARK_PROFILER         "Enable coz profiler" Off)
option(ARK_PROFILER_COUNT   "Enable creations/copies/moves counting on the Value" Off)
option(ARK_NO_STDLIB        "Do not install the standard library with the Ark library" Off)
option(ARK_BUILD_MODULES    "Build the std library modules or not" Off)

if (ARK_PROFILER_COUNT)
    target_compile_definitions(ArkReactor PRIVATE -DARK_PROFILER_COUNT)
endif()
if (ARK_PROFILER)
    target_compile_definitions(ArkReactor PRIVATE -DARK_PROFILER)
endif()
if (ARK_ENABLE_SYSTEM)
    target_compile_definitions(ArkReactor PRIVATE -DARK_ENABLE_SYSTEM)
endif()
if (ARK_BUILD_MODULES)
    add_subdirectory(${ark_SOURCE_DIR}/lib/modules)
endif()


if (ARK_BUILD_EXE)
    # additional files needed for the exe (repl, command line and stuff)
    set(EXE_SOURCES
        ${ark_SOURCE_DIR}/src/arkscript/REPL/Utils.cpp
        ${ark_SOURCE_DIR}/src/arkscript/REPL/Repl.cpp
        ${ark_SOURCE_DIR}/src/arkscript/main.cpp)

    add_executable(arkscript ${EXE_SOURCES})
    add_executable(ark ${EXE_SOURCES})

    add_subdirectory("${ark_SOURCE_DIR}/lib/replxx" EXCLUDE_FROM_ALL)
    add_subdirectory("${ark_SOURCE_DIR}/lib/clipp" EXCLUDE_FROM_ALL)

    target_include_directories(arkscript PUBLIC "${ark_SOURCE_DIR}/src/arkscript/")
    target_include_directories(ark PUBLIC "${ark_SOURCE_DIR}/src/arkscript/")

    target_link_libraries(arkscript PUBLIC ArkReactor replxx clipp termcolor)
    target_link_libraries(ark PUBLIC ArkReactor replxx clipp termcolor)

    set_target_properties(
        arkscript
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF)
    set_target_properties(
        ark
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF)

    # Installs the arkscript executable.
    install(TARGETS arkscript
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
