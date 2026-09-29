//
// Created by ivan on 6/25/26.
//
#include "AssetLoader/Refs/AssetRef.h"

#include "Render/RenderResourceManager.h"
#include "Systems/AudioSystem.h"


void RTGDEngine::AcquireAsset(MeshHandle h) {
    GRenderResources().AcquireAsset(h);
}

void RTGDEngine::ReleaseAsset(MeshHandle h) {
    GRenderResources().ReleaseAsset(h);
}

void RTGDEngine::AcquireAsset(MaterialHandle h) {
    GRenderResources().AcquireAsset(h);
}

void RTGDEngine::ReleaseAsset(MaterialHandle h) {
    GRenderResources().ReleaseAsset(h);
}

void RTGDEngine::AcquireAsset(TextureHandle h) {
    GRenderResources().AcquireAsset(h);
}

void RTGDEngine::ReleaseAsset(TextureHandle h) {
    GRenderResources().ReleaseAsset(h);
}

void RTGDEngine::AcquireAsset(BankHandle h)
{
    GAudio().AcquireAsset(h);
}

void RTGDEngine::ReleaseAsset(BankHandle h)
{
    GAudio().ReleaseAsset(h);
}
