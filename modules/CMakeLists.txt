cmake_minimum_required(VERSION 3.8)

project(Modules)

add_subdirectory(${PROJECT_SOURCE_DIR}/random)
add_subdirectory(${PROJECT_SOURCE_DIR}/console)

if (NOT DARK_BUILD_BASE_MODULES)
    add_subdirectory(${PROJECT_SOURCE_DIR}/sfml)
endif()