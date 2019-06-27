cmake_minimum_required(VERSION 3.8)

project(ArkBench)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#-------------------------------------------------------
#                      Benchmark lib
#-------------------------------------------------------
set(THIRD_PARTY_BENCHMARK_LIB ${CMAKE_CURRENT_BINARY_DIR}/third_party_benchmark/src/libbenchmark${CMAKE_STATIC_LIBRARY_SUFFIX})
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/third_party_benchmark)
add_custom_target(third_party_benchmark
        ALL
        COMMAND ${CMAKE_COMMAND} -DBENCHMARK_ENABLE_GTEST_TESTS=OFF ${ARK_SUBMODULES_ROOT}/benchmark -DCMAKE_BUILD_TYPE=Release
        COMMAND ${CMAKE_COMMAND} --build . --target benchmark
        COMMAND ${CMAKE_COMMAND} -E copy ${THIRD_PARTY_BENCHMARK_LIB} ${ARK_SUBMODULES_ROOT}/benchmark/lib/benchmark${CMAKE_STATIC_LIBRARY_SUFFIX}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/third_party_benchmark"
        COMMENT "Building benchmark"
        )

function(bench_make target_file)
    set(targetname bench_${target_file})

    add_executable(${targetname} ${target_file}.cpp)
    target_include_directories(${targetname} PRIVATE ${ARK_SUBMODULES_ROOT}/benchmark/include)
    target_link_libraries(${targetname} ArkLib)
    target_link_libraries(${targetname} "${ARK_SUBMODULES_ROOT}/benchmark/lib/benchmark${CMAKE_STATIC_LIBRARY_SUFFIX}" Threads::Threads)
    add_dependencies(${targetname} ArkLib)
    add_dependencies(${targetname} third_party_benchmark)
    set_target_properties(${targetname} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${Ark_SOURCE_DIR}/benchmark"
            OUTPUT_NAME  ${target_file})
    set_target_properties(
        ${targetname}
                PROPERTIES
                    CXX_STANDARD 17
                    CXX_STANDARD_REQUIRED ON
                    CXX_EXTENSIONS OFF
            )
endfunction()

bench_make(vm)