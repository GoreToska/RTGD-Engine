//
// Created by ivan on 8/18/26.
//

#ifdef __linux__
#include "Platform/Linux/LinuxDynamicLibrary.h"

#include <dlfcn.h>

#include "Tools/Logger.h"

namespace RTGDEngine {
    LinuxDynamicLibrary::~LinuxDynamicLibrary() {
        LinuxDynamicLibrary::Unload();
    }

    bool LinuxDynamicLibrary::Load(const std::string &path) {
        Unload();

        m_path = path;

        m_handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);

        if (!m_handle) {
            LogError("dlopen failed for '{}': {}", path, dlerror());
            return false;
        }

        return true;
    }

    void LinuxDynamicLibrary::Unload() {
        if (m_handle) {
            dlclose(m_handle);
            m_handle = nullptr;
        }
    }

    void *LinuxDynamicLibrary::GetSymbol(const std::string &name) const {
        return m_handle ? dlsym(m_handle, name.c_str()) : nullptr;
    }

    bool LinuxDynamicLibrary::IsLoaded() const {
        return m_handle != nullptr;
    }
} // RTGDEngine

#endif
