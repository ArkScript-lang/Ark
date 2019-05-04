cmake_minimum_required(VERSION 3.8)

project(random)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../../thirdparty
    ${PROJECT_SOURCE_DIR}/../../include
)

file(GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/../../thirdparty/*.cpp
    ${PROJECT_SOURCE_DIR}/../../thirdparty/*.c
    ${PROJECT_SOURCE_DIR}/../../src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

if (UNIX OR LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})
endif()

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

add_custom_command(TARGET ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:${PROJECT_NAME}>" ${PROJECT_SOURCE_DIR}/../../lib
)
