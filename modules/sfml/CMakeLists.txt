cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0074 NEW)

project(sfml)

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg -g -no-pie")
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -s")
elseif (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/DWIN32 /D_WINDOWS /W3 /GR /EHa /MDd" CACHE STRING "Debug compilation options" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "/DWIN32 /D_WINDOWS /W3 /GR /EHa /Ox /Ob2 /Oi /Ot /Oy /MD" CACHE STRING "Release compilation options" FORCE)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath,/usr/local/lib")
elseif (CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -v")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

####### code required for the SFML
if (WIN32)
    set(SFML_STATIC_LIBRARIES TRUE)
endif()
find_package(SFML 2.5 COMPONENTS graphics window system network audio REQUIRED)
####### code required for the SFML

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/include
    ${Ark_SOURCE_DIR}/thirdparty
    ${Ark_SOURCE_DIR}/include
)

file(GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

list(REMOVE_ITEM SOURCE_FILES "${PROJECT_SOURCE_DIR}/../../src/main.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE ArkReactor)
####### code required for the SFML
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system sfml-network sfml-audio)
####### code required for the SFML

if (CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

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
        "$<TARGET_FILE:${PROJECT_NAME}>" ${PROJECT_SOURCE_DIR}/../../lib/${PROJECT_NAME}.bin
)
