//
// Created by ivan on 8/18/26.
//

#ifdef _WIN32
#include "Platform/Windows/WindowsDynamicLibrary.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    WindowsDynamicLibrary::~WindowsDynamicLibrary()
    {
        WindowsDynamicLibrary::Unload();
    }

    bool WindowsDynamicLibrary::Load(const std::string& fullPath)
    {
        Unload();

        m_path = fullPath;

        m_handle = LoadLibrary(fullPath.c_str());

        if (!m_handle)
        {
            LogError("Failed to load dynamic library {}.", fullPath);
            return false;
        }

        return true;
    }

    void WindowsDynamicLibrary::Unload()
    {
        if (m_handle)
        {
            FreeLibrary(m_handle);
            m_handle = nullptr;
        }
    }

    void* WindowsDynamicLibrary::GetSymbol(const std::string& name) const
    {
        return m_handle ? GetProcAddress(m_handle, name.c_str()) : nullptr;
    }

    bool WindowsDynamicLibrary::IsLoaded() const
    {
        return m_handle != nullptr;
    }
} // RTGDEditor

#endif
