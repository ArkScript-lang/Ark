# ArkScript lib needs to know if we are building an exe to enable/disable specific code for embedding
target_compile_definitions(ArkReactor PRIVATE ARK_BUILD_EXE=1)

# additional files needed for the exe (repl, command line and stuff)
file(GLOB_RECURSE EXE_SOURCES *.cpp)
if (NOT ARK_STATIC)
    list(APPEND EXE_SOURCES "${ark_SOURCE_DIR}/thirdparties/fmt/src/format.cc")
endif ()
add_executable(arkscript ${EXE_SOURCES})

target_include_directories(arkscript SYSTEM PUBLIC "${ark_SOURCE_DIR}/thirdparties/clipp/include")
target_link_libraries(arkscript PUBLIC ArkReactor replxx clipp)
target_compile_features(arkscript PRIVATE cxx_std_20)

if (ARK_UNITY_BUILD)
    set_target_properties(arkscript PROPERTIES
            UNITY_BUILD ON
            UNITY_BUILD_MODE BATCH
            UNITY_BUILD_BATCH_SIZE 16)
endif ()
set_target_properties(arkscript PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${ark_SOURCE_DIR}"
        VS_DEBUGGER_COMMAND "$<TARGET_FILE:arkscript>"
        BUILD_WITH_INSTALL_RPATH ON
        INSTALL_RPATH_USE_LINK_PATH ON)
set_target_rpath(arkscript)

enable_lto(arkscript)

# Installs the arkscript executable.
install(TARGETS arkscript
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
