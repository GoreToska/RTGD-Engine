//
// Created by ivan on 8/18/26.
//

#include "Platform/DynamicLibraryFactory.h"

#ifdef _WIN32
#include "Platform/Windows/WindowsDynamicLibrary.h"
#elif defined(__linux__)
#include "Platform/Linux/LinuxDynamicLibrary.h"
#endif


namespace RTGDEngine {
    std::unique_ptr<IDynamicLibrary> CreateDynamicLibrary() {
#ifdef _WIN32
        return std::make_unique<WindowsDynamicLibrary>();
#elif defined(__linux__)
        return std::make_unique<LinuxDynamicLibrary>();
#else
        static_assert(false, "Unsupported platform");
#endif
    }
} // RTGDEngine
