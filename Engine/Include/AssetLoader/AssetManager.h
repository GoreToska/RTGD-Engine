//
// Created by gorev on 17.03.2026.
//

#pragma once
#include <functional>
#include <string>

#include "Render/RenderHandle.h"
#include "Render/RenderResourceManager.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine {
    class AssetManager {
        DECLARE_SINGLETON(AssetManager);

    public:
        MeshHandle GetMesh(
            const std::string &absolutePath,
            std::function<void(MeshHandle)> onComplete = nullptr);

        MeshHandle GetMeshSync(const std::string &absolutePath);

        TextureHandle GetTexture(
            const std::string &path,
            bool isSRGB = true,
            std::function<void(TextureHandle)> onComplete = nullptr);

        TextureHandle GetTextureSync(const std::string &absolutePath, bool isSRGB = true);

        void AssignTexture(MaterialHandle material, ETextureSlot slot, const std::string &meshAbsPath,
                           bool srgb = true);

        const std::string &GetMeshPath(MeshHandle mesh) const;

        const std::string &GetTexturePath(TextureHandle texture) const;

    private:
        static std::string Normalize(const std::string &path);

        std::unordered_map<std::string, MeshHandle> m_meshByPath = {};
        std::unordered_map<MeshHandle, std::string> m_meshPathByHandle = {};
        std::unordered_map<std::string, TextureHandle> m_textureByPath = {};
        std::unordered_map<TextureHandle, std::string> m_textureHandleByPath = {};

        mutable std::mutex m_registryMutex = {};
    };
} // RTGDEngine
