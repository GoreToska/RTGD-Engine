//
// Created by ivan on 9/22/26.
//

#pragma once

#include <fmod_studio.hpp>
#include <string>
#include <functional>

#include "Engine/EngineExport.h"
#include "AssetLoader/AssetHandle.h"
#include "AssetLoader/Refs/AssetRef.h"
#include "Render/ResourcePool.h"
#include "Tools/Alias.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine
{
    enum class EStopMode
    {
        AllowFadeout,
        Immediate,
    };

    struct BankData
    {
        FMOD::Studio::Bank* bank = nullptr;
        std::string Path;
    };

    class ENGINE_API AudioEvent
    {
    public:
        AudioEvent() = default;

        explicit AudioEvent(FMOD::Studio::EventInstance* instance) : m_instance(instance)
        {
        }

        bool IsValid() const;

        bool IsPlaying() const;

        void Stop(EStopMode mode = EStopMode::Immediate);

        void SetPaused(bool paused);

        void SetParameter(const char* name, float value);

        void SetPosition(const Float3& position);

    private:
        FMOD::Studio::EventInstance* m_instance = nullptr;
    };

    class ENGINE_API AudioSystem
    {
        DECLARE_SINGLETON(AudioSystem);

    public:
        void Initialize();

        void Update(World& world, float deltaTime);

        void Shutdown();

        FMOD::Studio::System* GetStudioSystem() const { return m_studioSystem; }

        BankHandle LoadBank(const std::string& absolutePath, uint64_t assetID);

        bool IsAlive(BankHandle handle) const;

        void AcquireAsset(BankHandle handle);

        void ReleaseAsset(BankHandle handle);

        std::function<void(uint32_t)> OnBankDestroyed = {};

        AudioEvent Play(std::string_view event);

        AudioEvent Play(std::string_view event, const Float3& position);

        void PlayOneShot(std::string_view event);

        void PlayOneShot(std::string_view event, const Float3& position);

        void SetPlaying(bool playing);

        void StopAll(EStopMode mode = EStopMode::Immediate);

        void SetBusVolume(std::string_view bus, float volume);

        void SetBusPaused(std::string_view bus, bool paused);

        void SetMasterVolume(float volume);

        void SetPaused(bool paused);

        void SetGlobalParameter(std::string_view name, float value);

        float GetGlobalParameter(std::string_view name) const;

        AudioEvent StartSnapshot(std::string_view name);

        FMOD::Studio::EventInstance* CreateInstance(std::string_view event, std::string_view prefix = "event:/");

    private:
        void ProcessPendingBankDestroys();

        FMOD::Studio::Bank* LoadBankFile(const std::string& absolutePath);

        FMOD::Studio::System* m_studioSystem = nullptr;
        FMOD::System* m_coreSystem = nullptr;

        std::vector<BankRef> m_bootstrapBanks = {};
        std::vector<AudioEvent> m_activeSnapshots = {};

        ResourcePool<BankData> m_banks = {};
        std::mutex m_lifetimeMutex = {};
        bool m_isPlaying = false;
    };

    DECLARE_GLOBAL_SINGLETON(AudioSystem, GAudio);
} // RTGDEngine
