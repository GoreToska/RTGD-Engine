//
// Created by ivan on 6/10/26.
//

#pragma once
#include "IPlatformWindow.h"
#include <memory>


namespace RTGDEngine {
    std::unique_ptr<IPlatformWindow> CreatePlatformWindow();
}
