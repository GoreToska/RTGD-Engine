//
// Created by ivan on 6/24/26.
//

#pragma once
#include <string>

#include "Render/RenderHandle.h"

namespace RTGDEngine {
    template<typename AssetHandle>
    struct AssetRef {
        std::string Path;
        AssetHandle Handle = {};

        AssetRef() = default;

        AssetRef(std::string path) : Path(std::move(path)) {
        }

        AssetRef(AssetHandle handle) : Handle(handle) {
        }

        [[nodiscard]] bool IsResolved() const {
            return Handle.IsValid();
        }
    };

    using MeshRef = AssetRef<MeshHandle>;
    using MaterialRef = AssetRef<MaterialHandle>;
}
