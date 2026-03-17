//
// Created by gorev on 17.03.2026.
//

#pragma once
#include <functional>
#include <string>

#include "Render/RenderHandle.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine
{
    class AssetLoader
    {
        DECLARE_SINGLETON(AssetLoader);

    public:
        MeshHandle LoadMeshAsync(
            const std::string& absolutePath,
            std::function<void(MeshHandle)> onComplete = nullptr);

        MeshHandle LoadMeshSync(const std::string& absolutePath);
    };
} // RTGDEngine
