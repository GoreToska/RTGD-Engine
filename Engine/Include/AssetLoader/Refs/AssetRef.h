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
            if (Handle.IsValid()) AcquireAsset(Handle);
        }

        AssetRef &operator=(const AssetRef &o) {
            if (this == &o) return *this;

            if (o.Handle.IsValid()) AcquireAsset(o.Handle);
            if (Handle.IsValid()) ReleaseAsset(Handle);

            Path = o.Path;
            Handle = o.Handle;
            return *this;
        }

        AssetRef(const AssetRef &o) : Path(o.Path), Handle(o.Handle) {
            if (Handle.IsValid()) AcquireAsset(Handle);
        }

        AssetRef(AssetRef &&o) noexcept : Path(std::move(o.Path)), Handle(o.Handle) {
            o.Handle = AssetHandle{};
        }

        AssetRef &operator=(AssetRef &&o) noexcept {
            if (this == &o) return *this;

            if (Handle.IsValid()) ReleaseAsset(Handle);

            Path = std::move(o.Path);
            Handle = o.Handle;
            o.Handle = AssetHandle{};
            return *this;
        }

        ~AssetRef() {
            if (Handle.IsValid()) ReleaseAsset(Handle);
        }

        void Resolve(AssetHandle handle) {
            if (handle == Handle) return;

            if (handle.IsValid()) AcquireAsset(handle);
            if (Handle.IsValid()) ReleaseAsset(Handle);
            Handle = handle;
        }

        [[nodiscard]] bool IsResolved() const {
            return Handle.IsValid();
        }
    };

    using MeshRef = AssetRef<MeshHandle>;
    using MaterialRef = AssetRef<MaterialHandle>;
    using TextureRef = AssetRef<TextureHandle>;

    void AcquireAsset(MeshHandle h);

    void ReleaseAsset(MeshHandle h);

    void AcquireAsset(MaterialHandle h);

    void ReleaseAsset(MaterialHandle h);

    void AcquireAsset(TextureHandle h);

    void ReleaseAsset(TextureHandle h);
}
