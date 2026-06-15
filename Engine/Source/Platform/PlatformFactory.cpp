//
// Created by ivan on 6/10/26.
//

#include "Platform/PlatformFactory.h"

#ifdef _WIN32
  #include "Platform/Windows/WindowsPlatform.h"
#elif defined(__linux__)
  #include "Platform/Linux/LinuxPlatform.h"
#endif

namespace RTGDEngine {
  std::unique_ptr<IPlatformWindow> CreatePlatformWindow() {
#ifdef _WIN32
    return std::make_unique<WindowsPlatform>();
#elif defined(__linux__)
    return std::make_unique<LinuxPlatform>();
#else
    static_assert(false, "Unsupported platform");
#endif
  }
}