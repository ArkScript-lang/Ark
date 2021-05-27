cmake_minimum_required(VERSION 3.8)

project(integration CXX)

add_subdirectory(${PROJECT_SOURCE_DIR}/../../ ${PROJECT_SOURCE_DIR}/ark/)
# create special directory for our executables
set(OUT_DIR ${PROJECT_SOURCE_DIR}/out)
file(MAKE_DIRECTORY ${OUT_DIR})

set(TARGET_LIST "01;02")

foreach(ELEM ${TARGET_LIST})
    set(FNAME ${ELEM}-test)

    add_executable(${FNAME} "${ELEM}.cpp")
    target_link_libraries(${FNAME} PUBLIC ArkReactor)
    target_include_directories(${FNAME} PUBLIC ${PROJECT_SOURCE_DIR}/ark/include)

    # copy to a special folder
    add_custom_command(
        TARGET ${FNAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${FNAME}> ${OUT_DIR}
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