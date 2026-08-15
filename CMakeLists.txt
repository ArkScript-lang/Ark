cmake_minimum_required(VERSION 3.16)

project(ark CXX C)

# ArkScript version (have to manually update it)
set(ARK_VERSION_MAJOR 4)
set(ARK_VERSION_MINOR 7)
set(ARK_VERSION_PATCH 3)
set(ARK_VERSION_PRERELEASE 1)

set(ARK_VERSION_STR "${ARK_VERSION_MAJOR}.${ARK_VERSION_MINOR}.${ARK_VERSION_PATCH}")

# determine whether this is a standalone project or included by other projects
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    execute_process(
            COMMAND git rev-parse --short=8 HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ARK_COMMIT
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(
            COMMAND date +%Y-%m-%dT%H:%M:%SZ
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ARK_BUILD_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE)
else ()
    set(ARK_COMMIT "deadbeef")
    set(ARK_BUILD_DATE "2026-01-02T17:56:00Z")
endif ()

# needed so that cppcheck has a cache folder, when launched through pre-commit
file(MAKE_DIRECTORY cache-cppcheck)

option(ARK_BUILD_EXE "Build a standalone arkscript executable" Off)
option(ARK_ENABLE_SYSTEM "Enable sys:exec" On)  # enable use of (sys:exec "command here")
option(ARK_STATIC "Build ArkScript statically" Off)
option(ARK_NO_STDLIB "Do not install the standard library with the Ark library" Off)
option(ARK_BUILD_MODULES "Build the std library modules or not" Off)
option(ARK_SANITIZERS "Enable ASAN and UBSAN" Off)
option(ARK_TESTS "Build ArkScript unit tests" Off)
option(ARK_BENCHMARKS "Build ArkScript benchmarks" Off)
option(ARK_COVERAGE "Enable coverage while building (clang, gcc) (requires ARK_TESTS to be On)" Off)
option(ARK_UNITY_BUILD "Enable unity build" Off)
option(ARK_JS_ONLY "Compiles to native JS (No WASM). Can be used only when compiling with emsdk" OFF)
option(ARK_DISTRIBUTE "Load configuration to distribute ArkScript (as .deb, .rpm, .dmg...)" OFF)

include(cmake/vars.cmake)
if (CMAKE_BUILD_TYPE STREQUAL "")
    message(WARNING "CMAKE_BUILD_TYPE was not specified, defaulting to 'Debug'")
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

if (NOT ARK_EMSCRIPTEN)
    include(cmake/link_time_optimization.cmake)
    include(cmake/sanitizers.cmake)
    include(GNUInstallDirs)  # Uses GNU Install directory variables
endif ()

include(cmake/CPM.cmake)
message(STATUS "ArkScript version ${ARK_VERSION_MAJOR}.${ARK_VERSION_MINOR}.${ARK_VERSION_PATCH}-${ARK_COMMIT}")

#####################################################
# Configure files
#####################################################

## installer.iss
configure_file(
        ${ark_SOURCE_DIR}/Installer.iss.in
        ${ark_SOURCE_DIR}/Installer.iss)
## Ark/Constants.hpp
configure_file(
        ${ark_SOURCE_DIR}/include/Ark/Constants.hpp.in
        ${ark_SOURCE_DIR}/include/Ark/Constants.hpp)

#####################################################
# Compilations options and project declaration
#####################################################

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

file(GLOB_RECURSE SOURCE_FILES
        ${ark_SOURCE_DIR}/src/arkreactor/*.cpp
        ${ark_SOURCE_DIR}/thirdparties/fmt/src/format.cc)

if (NOT ARK_EMSCRIPTEN AND NOT ARK_STATIC)
    add_library(ArkReactor SHARED ${SOURCE_FILES})
    enable_lto(ArkReactor)
    set_target_properties(ArkReactor PROPERTIES
            INSTALL_NAME_DIR "@rpath"
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_BINDIR})
else ()
    add_library(ArkReactor STATIC ${SOURCE_FILES})
    target_compile_definitions(ArkReactor PRIVATE ARK_STATIC)
endif ()

target_include_directories(ArkReactor
        PUBLIC
        ${ark_SOURCE_DIR}/include)
target_include_directories(ArkReactor
        SYSTEM PUBLIC
        "${ark_SOURCE_DIR}/thirdparties/picosha2/"
        "${ark_SOURCE_DIR}/thirdparties/fmt/include")

target_compile_features(ArkReactor PRIVATE cxx_std_20)
target_compile_definitions(ArkReactor PRIVATE ARK_EXPORT)

if (ARK_ENABLE_SYSTEM)
    target_compile_definitions(ArkReactor PRIVATE ARK_ENABLE_SYSTEM)
endif ()

if (ARK_UNITY_BUILD)
    set_target_properties(ArkReactor PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE BATCH UNITY_BUILD_BATCH_SIZE 16)
    set_source_files_properties(src/arkreactor/VM/VM.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION true)
endif ()

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR APPLE)
    message(STATUS "Enabling computed gotos")
    target_compile_definitions(ArkReactor PRIVATE ARK_USE_COMPUTED_GOTOS=1)

    target_compile_options(ArkReactor
            PRIVATE
            -Wall -Wextra -pedantic -Wstrict-aliasing
            -Wshadow
            -Wconversion
            # Allow deprecation warnings to not be treated as errors
            -Wno-error=deprecated-declarations
            # Allow command line flags to not be treated as errors when unused
            -Wno-unused-command-line-argument
            # We use pragmas to disable warnings we understand.
            # So we need to disable the warning about pragmas.
            -Wno-unknown-pragmas
            # Disable warnings about disabling warnings we have disabled.
            -Wno-unknown-warning-option
            # For our computed gotos needs
            -Wno-gnu-label-as-value
            # enable optimization inside the shared lib
            -fvisibility=hidden
            -fno-semantic-interposition
    )
    if (NOT ARK_BENCHMARKS AND NOT ARK_EMSCRIPTEN)
        target_compile_options(ArkReactor PRIVATE -Werror)
    endif ()

    # set fmtlib visibility to default
    target_compile_definitions(ArkReactor PUBLIC FMT_SHARED)

    if (APPLE)
        # The standard SSH libraries are depreciate on APPLE.
        # Thus they currently generate a warning that we have to ignore for now.
        # Though we should look to fix that in the future.
        target_compile_options(ArkReactor PRIVATE -Wno-deprecated-declarations)
    endif ()

    # The nlohmann/json.hpp external project has unused typedefs in the code
    # to compensate for this error we remove the following warning.
    if (CMAKE_COMPILER_IS_CLANG)
        target_compile_options(ArkReactor PRIVATE -Wno-unused-local-typedef)
    elseif (CMAKE_COMPILER_IS_GNUCXX)
        target_compile_options(ArkReactor PRIVATE -Wno-unused-local-typedefs)
    endif ()
elseif (MSVC)
    target_compile_definitions(ArkReactor PRIVATE ARK_USE_COMPUTED_GOTOS=0)
    target_compile_options(ArkReactor
            PRIVATE
            /W4
            /MP4      # build multiple source files concurrently
            /EHa      # set exception model to standard C++ stack unwinding
            /wd4267   # disable warning about data loss (size_t -> int)
            /wd4244   # disable warning about data loss (size_t -> char)
            /wd4505   # disable warning about unused static function was deleted
            /wd4068   # disable warnings about unknown pragmas.
    )
    target_compile_options(ArkReactor PUBLIC /utf-8)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:8000000")  # set stack size to 8MB
endif ()

#####################################################
# Link libraries
#####################################################

add_subdirectory("${ark_SOURCE_DIR}/thirdparties/ankerl_unordered_dense" EXCLUDE_FROM_ALL)
target_link_libraries(ArkReactor PUBLIC unordered_dense::unordered_dense)

add_library(newlib STATIC ${ark_SOURCE_DIR}/thirdparties/newlib/src/gmtime_r.c)
target_include_directories(newlib SYSTEM PUBLIC ${ark_SOURCE_DIR}/thirdparties/newlib/include)
target_include_directories(ArkReactor SYSTEM PUBLIC ${ark_SOURCE_DIR}/thirdparties/newlib/include)
target_link_libraries(ArkReactor PRIVATE newlib)

if (UNIX OR LINUX)
    find_package(Threads)
    target_link_libraries(ArkReactor PRIVATE ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif ()

#####################################################
# Installation rules
#####################################################

if (NOT ARK_EMSCRIPTEN)
    # Installs the dynamic library file.
    install(TARGETS ArkReactor
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

    # Install header files
    install(DIRECTORY ${ark_SOURCE_DIR}/include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    # Install the standard library
    if (NOT ARK_NO_STDLIB)
        install(DIRECTORY ${ark_SOURCE_DIR}/lib/std/
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/Ark/std
                FILES_MATCHING PATTERN "*.ark"
                PATTERN "std/tests" EXCLUDE
                PATTERN "std/.github" EXCLUDE
                PATTERN "scripts" EXCLUDE)
    endif ()
endif ()

#####################################################
# Create the different (optional) targets
#####################################################

if (ARK_BUILD_MODULES AND NOT ARK_STATIC)
    get_directory_property(old_dir_compile_options COMPILE_OPTIONS)

    add_compile_options(-w)
    add_subdirectory(${ark_SOURCE_DIR}/lib/modules)

    set_directory_properties(PROPERTIES COMPILE_OPTIONS "${old_dir_compile_options}")
endif ()

if (ARK_TESTS OR ARK_BUILD_EXE)
    add_subdirectory("${ark_SOURCE_DIR}/thirdparties/replxx" EXCLUDE_FROM_ALL)
    add_subdirectory("${ark_SOURCE_DIR}/thirdparties/clipp" EXCLUDE_FROM_ALL)
endif ()

if (ARK_EMSCRIPTEN)
    add_subdirectory(${ark_SOURCE_DIR}/src/arkemscripten)
endif ()

if (ARK_TESTS)
    set(BOOST_UT_DISABLE_MODULE On)
    add_subdirectory(${ark_SOURCE_DIR}/thirdparties/ut)
    add_subdirectory(${ark_SOURCE_DIR}/tests/unittests)
endif ()

if (ARK_BENCHMARKS)
    add_subdirectory(${ark_SOURCE_DIR}/tests/benchmarks)
endif ()

if (ARK_BUILD_EXE)
    add_subdirectory(${ark_SOURCE_DIR}/src/arkscript)
endif ()

if (ARK_SANITIZERS AND NOT ARK_STATIC)
    message(STATUS "Enabling ASAN and UBSAN")
    add_address_sanitizer()
    add_undefined_sanitizer()
endif ()

if (ARK_DISTRIBUTE)
    include(cmake/packaging.cmake)
    include(CPack)
endif ()
