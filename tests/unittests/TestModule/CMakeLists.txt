cmake_minimum_required(VERSION 3.16)

project(testmodule)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(${PROJECT_NAME} SHARED ${testmodule_SOURCE_DIR}/TestModule.cpp)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${ark_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE ArkReactor)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:${PROJECT_NAME}>" ${ark_SOURCE_DIR}/tests/unittests/${PROJECT_NAME}.arkm)
