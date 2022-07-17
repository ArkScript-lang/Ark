# MIT License
# 
# Copyright (c) 2020 Alan de Freitas
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Code from https://github.com/alandefreitas/moderncpp/blob/master/cmake/functions/sanitizers.cmake

# @brief Add sanitizer flag for Clang and GCC to all targets
# - You shouldn't use sanitizers in Release Mode
# - It's usually best to do that per target
macro(add_sanitizer flag)
    #[add_sanitizer Add sanitizer flag to all targets
    include(CheckCXXCompilerFlag)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        message("Looking for -fsanitize=${flag}")

        set(CMAKE_REQUIRED_FLAGS "-Werror -fsanitize=${flag}")
        check_cxx_compiler_flag(-fsanitize=${flag} HAVE_FLAG_SANITIZER)

        if (HAVE_FLAG_SANITIZER)
            message("Adding -fsanitize=${flag}")

            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=${flag} -fno-omit-frame-pointer")
            set(DCMAKE_C_FLAGS "${DCMAKE_C_FLAGS} -fsanitize=${flag} -fno-omit-frame-pointer")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=${flag}")
            set(DCMAKE_MODULE_LINKER_FLAGS "${DCMAKE_MODULE_LINKER_FLAGS} -fsanitize=${flag}")
        else()
            message("-fsanitize=${flag} unavailable")
        endif()
    endif()
    #]
endmacro()

# @brief Add address sanitizer to all targets
# - You shouldn't use sanitizers in Release Mode
# - It's usually best to do that per target
macro(add_address_sanitizer)
    #[add_address_sanitizer Add address sanitizer to all targets
    add_sanitizer("address")
    #]
endmacro()

# @brief Add thread sanitizer to all targets
# - You shouldn't use sanitizers in Release Mode
# - It's usually best to do that per target
macro(add_thread_sanitizer)
    #[add_thread_sanitizer Add thread sanitizer to all targets
    add_sanitizer("thread")
    #]
endmacro()

# @brief Add undefined sanitizer to all targets
# - You shouldn't use sanitizers in Release Mode
# - It's usually best to do that per target
macro(add_undefined_sanitizer)
    #[add_undefined_sanitizer Add undefined sanitizer to all targets
    add_sanitizer("undefined")
    #]
endmacro()

# @brief Add memory sanitizer to all targets
# - You shouldn't use sanitizers in Release Mode
# - It's usually best to do that per target
macro(add_memory_sanitizer)
    #[add_memory_sanitizer Add memory sanitizer to all targets
    add_sanitizer("memory")
    #]
endmacro()

# @brief @brief Add leak sanitizer to all targets
# - You shouldn't use sanitizers in Release Mode
# - It's usually best to do that per target
macro(add_leak_sanitizer)
    #[add_leak_sanitizer Add leak sanitizer to all targets
    add_sanitizer("leak")
    #]
endmacro()

# @brief Add all sanitizers to all targets
# - You shouldn't use sanitizers in Release Mode
# - It's usually best to do that per target
macro(add_sanitizers)
    #[add_sanitizers Add all sanitizers to all targets
    # Choose a subset of sanitizers not in conflict
    add_address_sanitizer()
    add_leak_sanitizer()
    add_undefined_sanitizer()
    # not allowed with address sanitizer
    # add_thread_sanitizer()
    # not supported
    # add_memory_sanitizer()
    #]
endmacro()
