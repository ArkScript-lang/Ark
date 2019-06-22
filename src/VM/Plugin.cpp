#include <Ark/VM/Plugin.hpp>

#include <sstream>
#include <iomanip>
#include <iostream>

namespace Ark
{
    namespace internal
    {
        SharedLibrary::SharedLibrary() :
            m_hInstance(NULL)
            , m_path("")
            , m_loaded(false)
        {}

        SharedLibrary::SharedLibrary(const std::string& path) :
            m_hInstance(NULL)
            , m_path(path)
            , m_loaded(false)
        {
            load(m_path);
        }

        SharedLibrary::~SharedLibrary()
        {
            unload();
        }

        void SharedLibrary::load(const std::string& path)
        {
            if (m_loaded)
                unload();
            
            m_path = path;
            
#if defined(_WIN32) || defined(_WIN64)
            if (NULL == (m_hInstance = LoadLibrary(m_path.c_str())))
            {
                throw std::system_error(
                    std::error_code(::GetLastError(), std::system_category())
                    , "Couldn't load the library"
                );
            }
#elif (defined(unix) || defined(__unix) || defined(__unix__)) || defined(__APPLE__)
            if (NULL == (m_hInstance = dlopen(m_path.c_str(), RTLD_LAZY | RTLD_GLOBAL)))
            {
                throw std::system_error(
                    std::error_code(errno, std::system_category())
                    , "Couldn't load the library, " + std::string(dlerror())
                );
            }
#endif
            m_loaded = true;
        }

        void SharedLibrary::unload()
        {
            if (m_loaded)
            {
#if defined(_WIN32) || defined(_WIN64)
                FreeLibrary(m_hInstance);
#elif (defined(unix) || defined(__unix) || defined(__unix__)) || defined(__APPLE__)
                dlclose(m_hInstance);
#endif
            }
        }

    }
} 
