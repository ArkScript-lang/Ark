/**
 * @file Plugin.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Loads .dll/.so/.dynlib files
 * @version 1.0
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef ARK_VM_PLUGIN_HPP
#define ARK_VM_PLUGIN_HPP

#include <Ark/Platform.hpp>

#if defined(ARK_OS_WINDOWS)
// do not include winsock.h
#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <Windows.h>
#elif defined(ARK_OS_LINUX)
#    include <dlfcn.h>
#else
#    error "Can not identify the platform on which you are running, aborting"
#endif

#include <string>
#include <system_error>

#include <fmt/core.h>

namespace Ark::internal
{
    /**
     * @brief Handling a shared library as an ArkScript plugin
     *
     */
    class SharedLibrary
    {
    public:
        /**
         * @brief Construct a new Shared Library object
         *
         */
        SharedLibrary();

        /**
         * @brief Disable copy semantics as this contains a pointer.
         *
         */
        SharedLibrary(const SharedLibrary&) = delete;

        /**
         * @brief Disable copy semantics as this contains a pointer.
         *
         */
        SharedLibrary& operator=(const SharedLibrary&) = delete;

        /**
         * @brief Construct a new Shared Library object
         *
         * @param path path to the shared library
         */
        explicit SharedLibrary(std::string path);

        /**
         * @brief Destroy the Shared Library object
         *
         */
        ~SharedLibrary();

        /**
         * @brief Load a shared library
         *
         * @param path path to the shared library
         */
        void load(const std::string& path);

        /**
         * @brief Unload the shared library
         *
         */
        void unload() const;

        [[nodiscard]] const std::string& path() const { return m_path; }

        /**
         * @brief Return a function from the shared library
         *
         * @tparam T the type of the function to retrieve
         * @param procname the name of the function to retrieve
         * @return T the function from the shared library, if it was found
         */
        template <typename T>
        T get(const std::string& procname)
        {
            T funcptr;

#if defined(ARK_OS_WINDOWS)
            if (NULL == (funcptr = reinterpret_cast<T>(GetProcAddress(m_instance, procname.c_str()))))
            {
                throw std::system_error(
                    std::error_code(::GetLastError(), std::system_category()), std::string("PluginError: Couldn't find ") + procname);
            }
#elif defined(ARK_OS_LINUX)
            if (NULL == (funcptr = reinterpret_cast<T>(dlsym(m_instance, procname.c_str()))))
            {
                throw std::system_error(
                    std::error_code(errno, std::system_category()), fmt::format("PluginError: Couldn't find {}, {}", procname, dlerror()));
            }
#endif
            return funcptr;
        }

    private:
#if defined(ARK_OS_WINDOWS)
        HINSTANCE m_instance;
#elif defined(ARK_OS_LINUX)
        void* m_instance;
#endif
        std::string m_path;
        bool m_loaded;
    };
}

#endif
