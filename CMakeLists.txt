cmake_minimum_required(VERSION 3.12)

project(ark CXX)

# VERSION
set(ARK_VERSION_MAJOR 4)
set(ARK_VERSION_MINOR 0)
set(ARK_VERSION_PATCH 0)

execute_process(
        COMMAND git rev-parse --short=8 HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ARK_COMMIT ${GIT_COMMIT_HASH})

include(cmake/link_time_optimization.cmake)
include(cmake/sanitizers.cmake)
include(GNUInstallDirs)  # Uses GNU Install directory variables

# configure installer.iss
configure_file(
    ${ark_SOURCE_DIR}/Installer.iss.in
    ${ark_SOURCE_DIR}/Installer.iss)

# setting up compilations options

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_CLANG ON)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# files needed for the library ArkReactor
file(GLOB_RECURSE SOURCE_FILES
    ${ark_SOURCE_DIR}/src/arkreactor/*.cpp
    ${ark_SOURCE_DIR}/lib/fmt/src/format.cc)

add_library(ArkReactor SHARED ${SOURCE_FILES})

target_compile_features(ArkReactor PRIVATE cxx_std_20)

enable_lto(ArkReactor)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR APPLE)
    target_compile_options(ArkReactor
        PUBLIC
        -Wall -Wextra -pedantic -Wstrict-aliasing
        -Wno-unknown-pragmas            # We use pragmas to disable warnings we understand.
                                        # So we need to disable the warning about pragmas.
        -Wno-unknown-warning-option     # Disable warnings about disabling warnings we have
                                        # disabled.
        )

    if (CMAKE_COMPILER_IS_GNUCXX)
        # The package utf8 has an issue with constant overflow.
        # Once this is fixed remove this flag:
        target_compile_options(ArkReactor PUBLIC -Wno-overflow)
    endif()

    if (APPLE)
        # The standard SSH libraries are depreciate on APPLE.
        # Thus they currently generate a warning that we have to ignore for now.
        # Though we should look to fix that in the future.
        target_compile_options(ArkReactor PUBLIC -Wno-deprecated-declarations)
    endif()

    # The following sub-modules link with -L/usr/local/lib
    # This causes a warning that is fatal. To prevent this
    # we disable this warning until this issue is fixed.
    #
    #   lib/modules/msgpack/CMakeFiles/msgpack.dir/__/submodules/msgpack-cpp/src/objectc.c.o
    #   lib/modules/msgpack/CMakeFiles/msgpack.dir/__/submodules/msgpack-cpp/src/version.c.o
    #   lib/modules/msgpack/CMakeFiles/msgpack.dir/__/submodules/msgpack-cpp/src/vrefbuffer.c.o
    #   lib/modules/msgpack/CMakeFiles/msgpack.dir/__/submodules/msgpack-cpp/src/zone.c.o
    target_compile_options(ArkReactor
        PUBLIC
        -Wno-unused-command-line-argument)

    # The nlohmann/json.hpp external project has unused typedefs in the code
    # to compensate for this error we remove the following warning.
    if (CMAKE_COMPILER_IS_CLANG OR APPLE)
        target_compile_options(ArkReactor PUBLIC -Wno-unused-local-typedef)
    elseif (CMAKE_COMPILER_IS_GNUCXX)
        target_compile_options(ArkReactor PUBLIC -Wno-unused-local-typedefs)
    endif()

elseif (MSVC)
    target_compile_options(ArkReactor
        PUBLIC
        /W4
        /MP4      # build multiple source files concurrently
        /EHa      # set exception model to standard C++ stack unwinding
        /wd4267   # disable warning about data loss (size_t -> int)
        /wd4244   # disable warning about data loss (size_t -> char)
        /wd4505   # disable warning about unused static function was deleted
        /wd4068)  # disable warnings about unknown pragmas.
endif()

# Link libraries

add_subdirectory("${ark_SOURCE_DIR}/lib/termcolor" EXCLUDE_FROM_ALL)

target_include_directories(ArkReactor
    PUBLIC
    "${ark_SOURCE_DIR}/lib/picosha2/"
    "${ark_SOURCE_DIR}/lib/fmt/include")

target_link_libraries(ArkReactor PUBLIC termcolor)

if (UNIX OR LINUX)
    find_package(Threads)
    target_link_libraries(ArkReactor PRIVATE ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
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

target_compile_features(ArkReactor PRIVATE cxx_std_20)

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
option(ARK_PROFILER_COUNT   "Enable creations/copies/moves counting on the Value" Off)
option(ARK_PROFILER_MIPS    "Enable MIPS calculation" Off)
option(ARK_NO_STDLIB        "Do not install the standard library with the Ark library" Off)
option(ARK_BUILD_MODULES    "Build the std library modules or not" Off)
option(ARK_SANITIZERS       "Enable ASAN and UBSAN" Off)
option(ARK_TESTS            "Build ArkScript unit tests" Off)

if (ARK_PROFILER_COUNT)
    target_compile_definitions(ArkReactor PRIVATE -DARK_PROFILER_COUNT)
endif()

if (ARK_ENABLE_SYSTEM)
    target_compile_definitions(ArkReactor PRIVATE -DARK_ENABLE_SYSTEM)
endif()

if (ARK_PROFILER_MIPS)
    target_compile_definitions(ArkReactor PRIVATE -DARK_PROFILER_MIPS)
endif()

if (ARK_BUILD_MODULES)
    add_subdirectory(${ark_SOURCE_DIR}/lib/modules)
endif()

# TODO: consider using ctest
if (ARK_TESTS)
    file(GLOB_RECURSE UT_SOURCES
            ${ark_SOURCE_DIR}/tests/unittests/*.cpp
            ${ark_SOURCE_DIR}/lib/fmt/src/format.cc
            ${ark_SOURCE_DIR}/src/arkscript/Formatter.cpp
            ${ark_SOURCE_DIR}/src/arkscript/JsonCompiler.cpp)
    add_executable(unittests ${UT_SOURCES})

    add_subdirectory(${ark_SOURCE_DIR}/lib/ut)
    target_link_libraries(unittests PUBLIC ArkReactor termcolor ut)

    add_compile_definitions(BOOST_UT_DISABLE_MODULE)
    target_compile_features(unittests PRIVATE cxx_std_20)
    target_compile_definitions(unittests PRIVATE ARK_TESTS_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/")
endif()

if (ARK_BUILD_EXE)
    # additional files needed for the exe (repl, command line and stuff)
    file(GLOB_RECURSE EXE_SOURCES
        ${ark_SOURCE_DIR}/src/arkscript/*.cpp
        ${ark_SOURCE_DIR}/lib/fmt/src/format.cc)
    add_executable(arkscript ${EXE_SOURCES})

    if (MSVC)
        # Disable warnings for lib/replxx
        # This is a separate module we don't control so can't fix the code without help.
        # Disable warnings when compiling this package remove when resolved.
        #   /wd4267:    conversion from 'size_t' to 'type', possible loss of data
        #   /wd4244:    conversion from 'type1' to 'type2', possible loss of data
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244")
    endif()

    add_subdirectory("${ark_SOURCE_DIR}/lib/replxx" EXCLUDE_FROM_ALL)
    add_subdirectory("${ark_SOURCE_DIR}/lib/clipp" EXCLUDE_FROM_ALL)

    target_link_libraries(arkscript PUBLIC ArkReactor replxx clipp termcolor)
    target_compile_features(arkscript PRIVATE cxx_std_20)

    enable_lto(arkscript)

    # Installs the arkscript executable.
    install(TARGETS arkscript
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (ARK_SANITIZERS)
    message(STATUS "Enabling address sanitizer and undefined behavior sanitizer")
    add_address_sanitizer()
    add_undefined_sanitizer()
endif()
