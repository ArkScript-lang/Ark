#include <Ark/VM/Plugin.hpp>

#include <sstream>
#include <iostream>

namespace Ark::internal
{
    SharedLibrary::SharedLibrary() :
        m_instance(nullptr),
        m_loaded(false)
    {}

    SharedLibrary::SharedLibrary(std::string path) :
        m_instance(nullptr),
        m_path(std::move(path)),
        m_loaded(false)
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

#if defined(ARK_OS_WINDOWS)
        if (NULL == (m_instance = LoadLibrary(m_path.c_str())))
        {
            throw std::system_error(
                std::error_code(::GetLastError(), std::system_category()), "Couldn't load the library at " + path);
        }
#elif defined(ARK_OS_LINUX)
        if (nullptr == (m_instance = dlopen(m_path.c_str(), RTLD_LAZY | RTLD_GLOBAL)))
        {
            throw std::system_error(
                std::error_code(errno, std::system_category()), "Couldn't load the library at " + path + ", " + std::string(dlerror()));
        }
#endif
        m_loaded = true;
    }

    void SharedLibrary::unload() const
    {
        if (m_loaded)
        {
#if defined(ARK_OS_WINDOWS)
            FreeLibrary(m_instance);
#elif defined(ARK_OS_LINUX)
            dlclose(m_instance);
#endif
        }
    }

}
