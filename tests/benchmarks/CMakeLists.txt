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

add_executable(bench main.cpp)
target_link_libraries(bench PUBLIC ArkReactor benchmark::benchmark)
target_compile_features(bench PRIVATE cxx_std_20)
target_compile_definitions(bench PRIVATE ARK_TESTS_ROOT="${ark_SOURCE_DIR}/")
enable_lto(bench)
