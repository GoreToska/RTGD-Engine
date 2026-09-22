//
// Created by ivan on 9/22/26.
//

#pragma once

#include <fmod_studio.hpp>
#include <string>
#include <functional>

#include "Engine/EngineExport.h"
#include "AssetLoader/AssetHandle.h"
#include "Render/ResourcePool.h"
#include "Tools/Alias.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine
{
    struct SoundData
    {
        FMOD::Studio::Bank* bank = nullptr;
        std::string Path;
    };

    class ENGINE_API AudioSystem
    {
        DECLARE_SINGLETON(AudioSystem);

    public:
        void Initialize();

        void Update(World& world, float deltaTime);

        void Shutdown();

        FMOD::Studio::System* GetStudioSystem() const { return m_studioSystem; }

        SoundHandle LoadSound(const std::string& absolutePath, uint64_t assetID);

        bool IsAlive(SoundHandle handle) const;

        void AcquireAsset(SoundHandle handle);

        void ReleaseAsset(SoundHandle handle);

        std::function<void(uint32_t)> OnSoundDestroyed = {};

    private:
        void ProcessPendingSoundDestroys();


        FMOD::Studio::System* m_studioSystem = nullptr;
        FMOD::System* m_coreSystem = nullptr;

        ResourcePool<SoundData> m_sounds = {};
        std::mutex m_lifetimeMutex = {};
    };

    DECLARE_GLOBAL_SINGLETON(AudioSystem, GAudio);
} // RTGDEngine
