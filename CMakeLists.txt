cmake_minimum_required(VERSION 3.8)

project(ark CXX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_CLANG ON)
endif()

# setting up compilations options

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg -g -no-pie")
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -s")
elseif (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Z7 /DWIN32 /D_WINDOWS /W3 /GR /EHa /MDd" CACHE STRING "Debug compilation options" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /DWIN32 /D_WINDOWS /W3 /GR /EHa /Ox /Ob2 /Oi /Ot /Oy /MD" CACHE STRING "Release compilation options" FORCE)
    add_compile_options(/wd4267)  # disable warning about data loss (size_t -> int)
    add_compile_options(/wd4244)  # disable warning about data loss (size_t -> char)
elseif (CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# submodules
add_subdirectory("${ark_SOURCE_DIR}/submodules/String")
# replxx
add_subdirectory("${ark_SOURCE_DIR}/submodules/replxx")
set(REPLXX_INCLUDE_DIR "${ark_SOURCE_DIR}/submodules/replxx/include")

# configuring Constants.hpp

# VERSION
set(ARK_VERSION_MAJOR  3)
set(ARK_VERSION_MINOR  0)
set(ARK_VERSION_PATCH 15)

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
set(ARK_SCOPE_DICHOTOMY Off CACHE BOOL "Change the behaviour of the scopes from linear (off) to dichotomic (On)")
set(ARK_BUILD_MODULES   Off CACHE BOOL "Build the std library modules or not")

if (ARK_SCOPE_DICHOTOMY)
    add_definitions(-DARK_SCOPE_DICHOTOMY)
endif()
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
    add_subdirectory(${ark_SOURCE_DIR}/lib/ext)
endif()

configure_file(
    ${ark_SOURCE_DIR}/include/Ark/Constants.hpp.in
    ${ark_SOURCE_DIR}/include/Ark/Constants.hpp
)

# files needed for the library ArkReactor
file(GLOB_RECURSE SOURCE_FILES
    ${ark_SOURCE_DIR}/src/Builtins/*.cpp
    ${ark_SOURCE_DIR}/src/Compiler/*.cpp
    ${ark_SOURCE_DIR}/src/Parser/*.cpp
    ${ark_SOURCE_DIR}/src/VM/*.cpp

    ${ark_SOURCE_DIR}/thirdparties/*.cpp
)
list(APPEND SOURCE_FILES
    ${ark_SOURCE_DIR}/src/Ark.cpp
    ${ark_SOURCE_DIR}/src/Utils.cpp
)

add_library(ArkReactor
    STATIC
        ${SOURCE_FILES}
        ${STRING_SOURCES}
)

# including content of project

target_include_directories(ArkReactor
    PUBLIC
        ${ark_SOURCE_DIR}/include
        ${ark_SOURCE_DIR}/thirdparties
        ${STRING_INCLUDES}
)

# include the profiler on demand
if (ARK_PROFILER)
    target_include_directories(ArkReactor PUBLIC ${Ark_SOURCE_DIR}/submodules/coz/include)
endif()

# linking libraries

find_package(Threads)

if (UNIX OR LINUX)
    if (CMAKE_COMPILER_IS_GNUCXX)
        target_link_libraries(ArkReactor PUBLIC stdc++fs)
    elseif (CMAKE_COMPILER_IS_CLANG)
        target_link_libraries(ArkReactor PUBLIC stdc++fs)
    endif()

    target_link_libraries(ArkReactor PUBLIC ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

# coz-profiler
if (ARK_PROFILER)
    find_package(coz-profiler REQUIRED PATHS ${Ark_SOURCE_DIR}/submodules/coz/)
    target_include_directories(ArkReactor PUBLIC ${COZ_INCLUDE_DIR})
    target_link_libraries(ArkReactor PUBLIC ${COZ_LIBRARY})
endif()

# setting up project properties

set_target_properties(
    ArkReactor
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

# building exe

if (ARK_BUILD_EXE)
    # additional files needed for the exe (repl, command line and stuff)
    set(EXE_SOURCES
        ${ark_SOURCE_DIR}/src/REPL/replxx/Util.cpp
        ${ark_SOURCE_DIR}/src/REPL/Repl.cpp
        ${ark_SOURCE_DIR}/src/main.cpp
    )

    include_directories(${REPLXX_INCLUDE_DIR})
    add_executable(ark ${EXE_SOURCES})
    target_include_directories(ark PUBLIC
        ${ark_SOURCE_DIR}/include
        ${ark_SOURCE_DIR}/thirdparties
    )
    target_link_libraries(ark PUBLIC ArkReactor replxx)

    set_target_properties(
        ark
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    # setting up installation directory

    install(TARGETS ark
            DESTINATION bin
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                        GROUP_EXECUTE GROUP_READ
    )
    install(DIRECTORY ${ark_SOURCE_DIR}/lib/
            DESTINATION lib
            PATTERN "lib/ext" EXCLUDE
            REGEX ".+\.md$" EXCLUDE
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                        GROUP_EXECUTE GROUP_READ
    )
endif()
