//
// Created by ivan on 6/25/26.
//
#include "AssetLoader/Refs/AssetRef.h"

#include "Render/RenderResourceManager.h"


void RTGDEngine::AcquireAsset(MeshHandle h) {
    RenderResourceManager::Instance().AcquireAsset(h);
}

void RTGDEngine::ReleaseAsset(MeshHandle h) {
    RenderResourceManager::Instance().ReleaseAsset(h);
}

void RTGDEngine::AcquireAsset(MaterialHandle h) {
    RenderResourceManager::Instance().AcquireAsset(h);
}

void RTGDEngine::ReleaseAsset(MaterialHandle h) {
    RenderResourceManager::Instance().ReleaseAsset(h);
}

void RTGDEngine::AcquireAsset(TextureHandle h) {
    RenderResourceManager::Instance().AcquireAsset(h);
}

void RTGDEngine::ReleaseAsset(TextureHandle h) {
    RenderResourceManager::Instance().ReleaseAsset(h);
}
