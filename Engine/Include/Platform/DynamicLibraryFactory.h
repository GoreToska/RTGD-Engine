//
// Created by ivan on 8/18/26.
//

#pragma once
#include "IDynamicLibrary.h"
#include <memory>

#include "Engine/EngineExport.h"

namespace RTGDEngine {
    ENGINE_API std::unique_ptr<IDynamicLibrary> CreateDynamicLibrary();
} // RTGDEngine
