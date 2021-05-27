cmake_minimum_required(VERSION 3.8)

project(integration CXX)

add_subdirectory(${PROJECT_SOURCE_DIR}../../ ${PROJECT_SOURCE_DIR}/ark)

set(TARGET_LIST "01;02")

foreach(ELEM ${TARGET_LIST})
    add_executable(${ELEM} "${ELEM}.cpp")
    target_link_libraries(${ELEM} PUBLIC ArkReactor)
    target_include_directories(${PROJECT_SOURCE_DIR}/ark/include)
    # request C++17
    set_target_properties(
        ${ELEM}
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )
endforeach()