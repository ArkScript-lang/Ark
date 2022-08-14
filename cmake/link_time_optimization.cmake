include(CheckIPOSupported)

check_ipo_supported(RESULT ipo_supported)

function(enable_lto target_name)
    if (ipo_supported)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND (CMAKE_CXX_COMPILER_VERSION MATCHES "^8\..+"))
            message(WARNING "LTO supported but not enabled to prevent https://github.com/ArkScript-lang/Ark/pull/385#issuecomment-1163597951")
        else()
            message(STATUS "LTO enabled")
            set_target_properties(
                ${target_name}
                PROPERTIES
                    INTERPROCEDURAL_OPTIMIZATION TRUE)
        endif()
    else()
        message(STATUS "LTO not supported")
    endif()
endfunction()
