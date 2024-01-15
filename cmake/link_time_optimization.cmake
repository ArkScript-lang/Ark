include(CheckIPOSupported)

check_ipo_supported(RESULT ipo_supported)

function(enable_lto target_name)
    if (ipo_supported AND (${CMAKE_BUILD_TYPE} STREQUAL "Release"))
        message(STATUS "LTO enabled")
        set_target_properties(${target_name} PROPERTIES
                INTERPROCEDURAL_OPTIMIZATION TRUE)
    else ()
        message(STATUS "LTO disabled outside Release mode")
    endif ()
endfunction()
