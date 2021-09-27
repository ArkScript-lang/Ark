cmake_minimum_required(VERSION 3.8)

project(ark CXX)

# VERSION
set(ARK_VERSION_MAJOR 3)
set(ARK_VERSION_MINOR 1)
set(ARK_VERSION_PATCH 1)

# Uses GNU Install directory variables
include(GNUInstallDirs)

# setting up compilations options

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_CLANG ON)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    set_property(GLOBAL PROPERTY ArkDebugFlags "${CMAKE_CXX_FLAGS_DEBUG} -pg -g -no-pie")
    set_property(GLOBAL PROPERTY ArkReleaseFlags "-DNDEBUG -O3 -s")
    set_property(GLOBAL PROPERTY ArkLinkerFlags "")
elseif (MSVC)
    set_property(GLOBAL PROPERTY ArkDebugFlags "/Z7 /DWIN32 /D_WINDOWS /W3 /GR /EHa /MDd")
    set_property(GLOBAL PROPERTY ArkReleaseFlags "/DNDEBUG /DWIN32 /D_WINDOWS /W3 /GR /EHa /Ox /Ob2 /Oi /Ot /Oy /MD")
    set_property(GLOBAL PROPERTY ArkLinkerFlags "")

    add_compile_options(/wd4267)  # disable warning about data loss (size_t -> int)
    add_compile_options(/wd4244)  # disable warning about data loss (size_t -> char)
elseif (CMAKE_COMPILER_IS_CLANG)
    set_property(GLOBAL PROPERTY ArkDebugFlags "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set_property(GLOBAL PROPERTY ArkReleaseFlags "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set_property(GLOBAL PROPERTY ArkLinkerFlags "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# files needed for the library ArkReactor
file(GLOB_RECURSE SOURCE_FILES
    ${ark_SOURCE_DIR}/src/arkreactor/*.cpp
)

add_library(ArkReactor
    SHARED
    ${SOURCE_FILES}
)


# Adds variant header project to includes.
target_include_directories(ArkReactor PUBLIC "${ark_SOURCE_DIR}/lib/variant/")

# Adds utf8_decoder header project to includes.
target_include_directories(ArkReactor PUBLIC "${ark_SOURCE_DIR}/lib/utf8_decoder/")

# Adds picosha2 header project to includes.
target_include_directories(ArkReactor PUBLIC "${ark_SOURCE_DIR}/lib/picosha2/")

# Adds termcolor source project for static linking
add_subdirectory("${ark_SOURCE_DIR}/lib/termcolor" EXCLUDE_FROM_ALL)
# Links to the target ArkReactor
target_link_libraries(ArkReactor PUBLIC termcolor)

# Adds String source project for static linking
add_subdirectory("${ark_SOURCE_DIR}/lib/String/" EXCLUDE_FROM_ALL)
# Adds String headers project to includes.
target_include_directories(ArkReactor PUBLIC "${ark_SOURCE_DIR}/lib/String/include/")
# Links to the target ArkReactor
target_link_libraries(ArkReactor PUBLIC ArkScriptString)


# coz-profiler
if (ARK_PROFILER)
    find_package(coz-profiler REQUIRED PATHS ${ark_SOURCE_DIR}/lib/coz/)
    target_include_directories(ArkReactor PUBLIC ${COZ_INCLUDE_DIR})
    target_link_libraries(ArkReactor PUBLIC ${COZ_LIBRARY})
endif()


# including content of project
target_include_directories(ArkReactor
    PUBLIC
    ${ark_SOURCE_DIR}/include
)

# setting up project properties
set_target_properties(
    ArkReactor
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

# Installs the dynamic library file.
install(TARGETS ArkReactor
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Install header files
install(DIRECTORY ${ark_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install the standard library
if(NOT ARK_NO_STD)
    install(DIRECTORY ${ark_SOURCE_DIR}/lib/std/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/Ark/std
        FILES_MATCHING PATTERN "*.ark"
        PATTERN "std/tests" EXCLUDE
        PATTERN "std/.github" EXCLUDE)
endif()


# set debug compilation flags
get_property(DebugFlags GLOBAL PROPERTY ArkDebugFlags)
if (DebugFlags)
    set(CMAKE_CXX_FLAGS_DEBUG "${DebugFlags}" CACHE STRING "Debug compilation options" FORCE)
endif()

# set release compilation flags
get_property(ReleaseFlags GLOBAL PROPERTY ArkReleaseFlags)
if (ReleaseFlags)
    set(CMAKE_CXX_FLAGS_RELEASE "${ReleaseFlags}" CACHE STRING "Release compilation options" FORCE)
endif()

# set linker flags
get_property(LinkerFlags GLOBAL PROPERTY ArkLinkerFlags)
if (LinkerFlags)
    set(CMAKE_EXE_LINKER_FLAGS "${LinkerFlags}" CACHE STRING "Linker options" FORCE)
endif()

# configuring Constants.hpp
message(STATUS "ArkScript version ${ARK_VERSION_MAJOR}.${ARK_VERSION_MINOR}.${ARK_VERSION_PATCH}")
if (ARK_PROFILER)
    message(STATUS "Coz Profiler is enabled")
else()
    message(STATUS "Coz Profiler is NOT enabled")
endif()

# COMPILATION RELATED
set(ARK_COMPILATION_OPTIONS ${CMAKE_CXX_FLAGS})
set(ARK_COMPILER            ${CMAKE_CXX_COMPILER_ID})

set(ARK_BUILD_EXE       Off CACHE BOOL "Build a standalone ark executable")
set(ARK_ENABLE_SYSTEM   On  CACHE BOOL "Enable sys:exec")  # enable use of (sys:exec "command here")
set(ARK_PROFILER        Off CACHE BOOL "Enable coz profiler")
set(ARK_PROFILER_COUNT  Off CACHE BOOL "Enable creations/copies/moves counting on the Value")
set(ARK_NO_STDLIB       Off CACHE BOOL "Do not install the standard library with the Ark library")
set(ARK_BUILD_MODULES   Off CACHE BOOL "Build the std library modules or not")


if (ARK_PROFILER_COUNT)
    add_definitions(-DARK_PROFILER_COUNT)
endif()
if (ARK_PROFILER)
    add_definitions(-DARK_PROFILER)
endif()
if (ARK_ENABLE_SYSTEM)
    add_definitions(-DARK_ENABLE_SYSTEM)
endif()
if (ARK_BUILD_MODULES)
    # submodules
    add_subdirectory(${ark_SOURCE_DIR}/lib/modules)
endif()

target_compile_definitions(ArkReactor PRIVATE ARK_EXPORT)

# linking libraries
find_package(Threads)

if (UNIX OR LINUX)
    if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
        target_link_libraries(ArkReactor PUBLIC stdc++fs)
    endif()
    target_link_libraries(ArkReactor PRIVATE ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()


configure_file(
    ${ark_SOURCE_DIR}/include/Ark/Constants.hpp.in
    ${ark_SOURCE_DIR}/include/Ark/Constants.hpp
)

if (ARK_BUILD_EXE)
    # additional files needed for the exe (repl, command line and stuff)
    set(EXE_SOURCES
        ${ark_SOURCE_DIR}/src/arkscript/REPL/Utils.cpp
        ${ark_SOURCE_DIR}/src/arkscript/REPL/Repl.cpp
        ${ark_SOURCE_DIR}/src/arkscript/main.cpp
    )

    add_executable(arkscript ${EXE_SOURCES})
    add_executable(ark ${EXE_SOURCES})

    # Adds replxx source project for static linking
    add_subdirectory("${ark_SOURCE_DIR}/lib/replxx" EXCLUDE_FROM_ALL)
    # Links to the target arkscript
    target_link_libraries(arkscript PUBLIC replxx)
    target_link_libraries(ark PUBLIC replxx)

    target_include_directories(arkscript PUBLIC "${ark_SOURCE_DIR}/src/arkscript/")
    target_include_directories(ark PUBLIC "${ark_SOURCE_DIR}/src/arkscript/")

    # Adds clipp source project for static linking
    add_subdirectory("${ark_SOURCE_DIR}/lib/clipp" EXCLUDE_FROM_ALL)
    # Links to the target arkscript
    target_link_libraries(arkscript PRIVATE clipp)
    target_link_libraries(ark PRIVATE clipp)

    # Links to the target arkscript
    target_link_libraries(arkscript PRIVATE termcolor)
    target_link_libraries(ark PRIVATE termcolor)

    set_target_properties(
        arkscript
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )
    set_target_properties(
        ark
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    target_link_libraries(arkscript PUBLIC ArkReactor)
    target_link_libraries(ark PUBLIC ArkReactor)

    # Installs the arkscript executable.
    install(TARGETS arkscript
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
