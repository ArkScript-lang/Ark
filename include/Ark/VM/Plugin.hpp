#ifndef ark_vm_plugin
#define ark_vm_plugin

#if defined(_WIN32) || defined(_WIN64)
    // do not include winsock.h
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
#elif (defined(unix) || defined(__unix) || defined(__unix__)) || defined(__APPLE__)
    #include <dlfcn.h>
#else
    #error "Can not identify the platform on which you are running, aborting"
#endif

#include <string>
#include <system_error>
#include <exception>

namespace Ark::internal
{
    class SharedLibrary
    {
    public:
        SharedLibrary();
        // loading a shared library from its path
        SharedLibrary(const std::string& path);
        ~SharedLibrary();

        void load(const std::string& path);
        void unload();

        template <typename T>
        T get(const std::string& procname)
        {
            T funcptr;

#if defined(_WIN32) || defined(_WIN64)
            if (NULL == (funcptr = reinterpret_cast<T>(GetProcAddress(m_hInstance, procname.c_str()))))
            {
                throw std::system_error(
                    std::error_code(::GetLastError(), std::system_category())
                    , std::string("Couldn't find ") + procname
                );
            }
#elif (defined(unix) || defined(__unix) || defined(__unix__)) || defined(__APPLE__)
            if (NULL == (funcptr = reinterpret_cast<T>(dlsym(m_hInstance, procname.c_str()))))
            {
                throw std::system_error(
                    std::error_code(errno, std::system_category())
                    , std::string("Couldn't find ") + procname + ", " + std::string(dlerror())
                );
            }
#endif
            return funcptr;
        }
    
    private:
#if defined(_WIN32) || defined(_WIN64)
        HINSTANCE m_hInstance;
#elif (defined(unix) || defined(__unix) || defined(__unix__)) || defined(__APPLE__)
        void* m_hInstance;
#endif
        std::string m_path;
        bool m_loaded;
    };
}


#endif