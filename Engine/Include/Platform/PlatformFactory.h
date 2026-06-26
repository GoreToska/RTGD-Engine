//
// Created by ivan on 6/10/26.
//

#pragma once
#include "IPlatformWindow.h"
#include <memory>

#include "Engine/EngineExport.h"


namespace RTGDEngine {
    ENGINE_API std::unique_ptr<IPlatformWindow> CreatePlatformWindow();

    ENGINE_API std::unique_ptr<IPlatformWindow> CreateEmbeddedPlatformWindow(const NativeWindowHandle &windowHandle);
}
