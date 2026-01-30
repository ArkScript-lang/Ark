if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_COMPILER_IS_CLANG ON)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set(ARK_EMSCRIPTEN TRUE)
endif ()

function(set_target_rpath target)
    if (APPLE)
        set_target_properties(${target} PROPERTIES
                INSTALL_RPATH "@executable_path;@executable_path/../lib;@executable_path/bin;@executable_path/lib")
    elseif (UNIX)
        set_target_properties(${target} PROPERTIES
                INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib:\$ORIGIN/bin:\$ORIGIN/lib")
    endif ()
endfunction()
