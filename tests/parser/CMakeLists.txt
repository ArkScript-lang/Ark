cmake_minimum_required(VERSION 3.20)

project(parser)

include(../../cmake/sanitizers.cmake)
option(ARK_SANITIZERS "Enable ASAN and UBSAN" Off)

find_library(ArkReactor
    NAMES
        ArkReactor
        libArkReactor
    PATHS
        "../../build/Release"
        "../../build"
        "../../build/Debug")

add_executable(parser main.cpp)

target_link_libraries(parser PUBLIC ${ArkReactor})
target_include_directories(
    parser
    PUBLIC
    ${PROJECT_SOURCE_DIR}/../../include
)

if (ARK_SANITIZERS)
    message(STATUS "Enabling address sanitizer and undefined behavior sanitizer")
    add_address_sanitizer()
    add_undefined_sanitizer()
endif()

target_compile_features(parser PRIVATE cxx_std_17)
