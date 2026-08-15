add_subdirectory(TestModule)

file(GLOB_RECURSE SOURCES
        *.cpp
        Suites/*.cpp
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

target_link_libraries(unittests PUBLIC ArkReactor ut replxx)

target_compile_features(unittests PRIVATE cxx_std_20)
target_compile_definitions(unittests PRIVATE ARK_TESTS_ROOT="${ark_SOURCE_DIR}/")

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
