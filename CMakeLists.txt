cmake_minimum_required(VERSION 3.16)

project(ark CXX C)

# ArkScript version (have to manually update it)
set(ARK_VERSION_MAJOR 4)
set(ARK_VERSION_MINOR 7)
set(ARK_VERSION_PATCH 2)
set(ARK_VERSION_PRERELEASE 0)

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
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/public")

    # ArkScript lib needs to know if we are building an exe to enable/disable specific code for embedding
    target_compile_definitions(ArkReactor PRIVATE ARK_BUILD_EXE=1)

    add_executable(ArkEmscripten ${ark_SOURCE_DIR}/src/arkemscripten/main.cpp)
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
endif ()

if (ARK_TESTS)
    add_subdirectory(${ark_SOURCE_DIR}/tests/unittests/TestModule)

    file(GLOB_RECURSE SOURCES
            ${ark_SOURCE_DIR}/tests/unittests/*.cpp
            ${ark_SOURCE_DIR}/tests/unittests/Suites/*.cpp
            ${ark_SOURCE_DIR}/src/arkscript/Formatter.cpp
            ${ark_SOURCE_DIR}/src/arkscript/JsonCompiler.cpp
            ${ark_SOURCE_DIR}/src/arkscript/REPL/Utils.cpp)
    if (NOT ARK_STATIC)
        list(APPEND SOURCES ${ark_SOURCE_DIR}/thirdparties/fmt/src/format.cc)
    endif ()
    add_executable(unittests ${SOURCES})
    target_include_directories(unittests PUBLIC ${ark_SOURCE_DIR}/tests/unittests)
    target_include_directories(unittests PUBLIC ${ark_SOURCE_DIR}/include)
    target_include_directories(unittests SYSTEM PUBLIC ${ark_SOURCE_DIR}/thirdparties/dtl/dtl)

    set(BOOST_UT_DISABLE_MODULE On)
    add_subdirectory(${ark_SOURCE_DIR}/thirdparties/ut)
    target_link_libraries(unittests PUBLIC ArkReactor ut replxx)

    target_compile_features(unittests PRIVATE cxx_std_20)
    target_compile_definitions(unittests PRIVATE ARK_TESTS_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/")

    if (ARK_UNITY_BUILD)
        set_target_properties(unittests PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE BATCH UNITY_BUILD_BATCH_SIZE 16)
    endif ()
    set_target_properties(unittests PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${ark_SOURCE_DIR}"
            VS_DEBUGGER_COMMAND "$<TARGET_FILE:unittests>"
            BUILD_WITH_INSTALL_RPATH ON
            INSTALL_RPATH_USE_LINK_PATH ON)
    set_target_rpath(unittests)

    if (ARK_COVERAGE AND CMAKE_COMPILER_IS_CLANG)
        target_compile_options(unittests PRIVATE -coverage -fcoverage-mapping -fprofile-instr-generate)
        target_link_options(unittests PRIVATE -coverage -fcoverage-mapping -fprofile-instr-generate)
        target_compile_options(ArkReactor PRIVATE -coverage -fcoverage-mapping -fprofile-instr-generate)
        target_link_options(ArkReactor PRIVATE -coverage -fcoverage-mapping -fprofile-instr-generate)

        # find required tools
        find_program(LCOV lcov REQUIRED)
        find_program(GENHTML genhtml REQUIRED)

        # add coverage target
        add_custom_target(coverage
                # gather data
                COMMAND ${LCOV} --directory . --capture
                --rc derive_function_end_line=0
                --ignore-errors inconsistent
                --filter range
                --exclude 'tests/*'
                --exclude 'thirdparties/*'
                --exclude '/usr/*'
                --exclude 'build/*'
                --gcov-tool ${ark_SOURCE_DIR}/tests/llvm-gcov.sh
                --output-file coverage.info
                # generate report
                COMMAND ${GENHTML}
                --demangle-cpp
                --rc derive_function_end_line=0
                -o coverage coverage.info
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif ()
endif ()

if (ARK_BENCHMARKS)
    set(BENCHMARK_ENABLE_GTEST_TESTS Off)
    set(BENCHMARK_ENABLE_TESTING Off)

    if (ARK_BENCHMARKS_CODSPEED)
        set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
        CPMAddPackage(
                URI "gh:CodSpeedHQ/codspeed-cpp#5324618a1a14e0aa9a99dcfb467b9c2a57957e19@1.1.1"
                SOURCE_SUBDIR google_benchmark)
    else ()
        CPMAddPackage("gh:google/benchmark@1.8.3")
    endif ()

    add_executable(bench tests/benchmarks/main.cpp)
    target_link_libraries(bench PUBLIC ArkReactor benchmark::benchmark)
    target_compile_features(bench PRIVATE cxx_std_20)
    target_compile_definitions(bench PRIVATE ARK_TESTS_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/")
    enable_lto(bench)
endif ()

if (ARK_BUILD_EXE)
    # ArkScript lib needs to know if we are building an exe to enable/disable specific code for embedding
    target_compile_definitions(ArkReactor PRIVATE ARK_BUILD_EXE=1)

    # additional files needed for the exe (repl, command line and stuff)
    file(GLOB_RECURSE EXE_SOURCES ${ark_SOURCE_DIR}/src/arkscript/*.cpp)
    if (NOT ARK_STATIC)
        list(APPEND EXE_SOURCES "${ark_SOURCE_DIR}/thirdparties/fmt/src/format.cc")
    endif ()
    add_executable(arkscript ${EXE_SOURCES})

    target_include_directories(arkscript SYSTEM PUBLIC "${ark_SOURCE_DIR}/thirdparties/clipp/include")
    target_link_libraries(arkscript PUBLIC ArkReactor replxx clipp)
    target_compile_features(arkscript PRIVATE cxx_std_20)

    if (ARK_UNITY_BUILD)
        set_target_properties(arkscript PROPERTIES
                UNITY_BUILD ON
                UNITY_BUILD_MODE BATCH
                UNITY_BUILD_BATCH_SIZE 16)
    endif ()
    set_target_properties(arkscript PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${ark_SOURCE_DIR}"
            VS_DEBUGGER_COMMAND "$<TARGET_FILE:arkscript>"
            BUILD_WITH_INSTALL_RPATH ON
            INSTALL_RPATH_USE_LINK_PATH ON)
    set_target_rpath(arkscript)

    enable_lto(arkscript)

    # Installs the arkscript executable.
    install(TARGETS arkscript
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

if (ARK_SANITIZERS AND NOT ARK_STATIC)
    message(STATUS "Enabling ASAN and UBSAN")
    add_address_sanitizer()
    add_undefined_sanitizer()
endif ()

include(cmake/packaging.cmake)
include(CPack)
