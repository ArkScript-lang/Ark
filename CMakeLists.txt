cmake_minimum_required(VERSION 3.8)

project(Ark CXX)

if (NOT DEBUG)
    message(STATUS "To enable debug mode, use -DDEBUG=true")
    message(STATUS "Falling back to release")
    
    set(DEBUG "false" CACHE STRING "Enables debug mode" FORCE)
endif()

if (NOT TESTCPP)
    message(STATUS "To use test.cpp instead of main.cpp, use -DTESTCPP=true")
    message(STATUS "Using main.cpp")
    set(TESTCPP "false" CACHE STRING "Use test.cpp instead of main.cpp" FORCE)
endif()

set(CMAKE_MODULE_PATH ${Ark_SOURCE_DIR}/cmake)
find_package(MPIR REQUIRED)

set(ARK_VERSION_MAJOR 1)
set(ARK_VERSION_MINOR 2)
set(ARK_VERSION_PATCH 2)
set(ARK_STD ${CMAKE_INSTALL_PREFIX}/share/.Ark/lib)

configure_file(
    ${Ark_SOURCE_DIR}/include/Ark/Constants.hpp.in
    ${Ark_SOURCE_DIR}/include/Ark/Constants.hpp
)

include_directories(
    ${Ark_SOURCE_DIR}/include
    ${Ark_SOURCE_DIR}/thirdparty

    ${MPIR_INCLUDE_DIR}
)

file(GLOB_RECURSE SOURCE_FILES
    ${Ark_SOURCE_DIR}/src/*.cpp
    ${Ark_SOURCE_DIR}/thirdparty/*.cpp
)

if (TESTCPP MATCHES "true")
    list(REMOVE_ITEM SOURCE_FILES "${Ark_SOURCE_DIR}/src/main.cpp")
else()
    list(REMOVE_ITEM SOURCE_FILES "${Ark_SOURCE_DIR}/src/test.cpp")
endif()

add_executable(Ark ${SOURCE_FILES})
target_link_libraries(Ark PRIVATE ${MPIR_LIBRARIES})
if (CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(Ark PRIVATE stdc++fs)
endif()

set_target_properties(
    Ark
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

if (DEBUG MATCHES "true")
    if (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "-pg -g -no-pie")
    endif()
endif()


install(TARGETS Ark DESTINATION bin)
install(DIRECTORY ${Ark_SOURCE_DIR}/lib/ DESTINATION share/.Ark/lib)