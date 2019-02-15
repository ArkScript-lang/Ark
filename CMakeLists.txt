cmake_minimum_required(VERSION 3.8)

project(Ark CXX)

include_directories(
    ${Ark_SOURCE_DIR}/include
    ${Ark_SOURCE_DIR}/thirdparty
)

file(GLOB_RECURSE SOURCE_FILES
    ${Ark_SOURCE_DIR}/src/*.cpp
    ${Ark_SOURCE_DIR}/thirdparty/*.cpp
)

add_executable(Ark ${SOURCE_FILES})

set_target_properties(
    Ark
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
