cmake_minimum_required(VERSION 3.8)

project(integration CXX)

include(../../cmake/sanitizers.cmake)
option(ARK_SANITIZERS "Enable ASAN and UBSAN" Off)

if (ARK_SANITIZERS)
    message(STATUS "Enabling address sanitizer and undefined behavior sanitizer")
    add_address_sanitizer()
    add_undefined_sanitizer()
endif()

# create special directory for our executables
set(OUT_DIR ${PROJECT_SOURCE_DIR}/out)
file(MAKE_DIRECTORY ${OUT_DIR})

find_library(ArkReactor
    NAMES
        ArkReactor
        libArkReactor
    PATHS
        "../../build/Release"
        "../../build"
        "../../build/Debug")

set(TARGET_LIST "01;02;03")

foreach(ELEM ${TARGET_LIST})
    set(FNAME ${ELEM}-test)

    add_executable(${FNAME} "${ELEM}.cpp")
    target_link_libraries(${FNAME} PUBLIC ${ArkReactor})
    target_include_directories(
        ${FNAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/../../include
        ${PROJECT_SOURCE_DIR}/../../lib/String/include
    )

    # copy to a special folder
    add_custom_command(
        TARGET ${FNAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${FNAME}> ${OUT_DIR}/${FNAME}
    )

    # request C++17
    set_target_properties(
        ${FNAME}
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )
endforeach()
