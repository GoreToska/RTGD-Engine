//
// Created by ivan on 9/29/26.
//

#include "Systems/AudioSystem.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    void AudioSystem::Initialize()
    {
        LogInfo("Audio disabled: built without FMOD.");
    }

    void AudioSystem::Update(World& world, float deltaTime)
    {
    }

    void AudioSystem::Shutdown()
    {
        LogInfo("Audio disabled: built without FMOD.");
    }

    BankHandle AudioSystem::LoadBank(const std::string& absolutePath, uint64_t assetID)
    {
        return INVALID_BANK_HANDLE;
    }

    AudioEvent AudioSystem::Play(std::string_view event)
    {
        LogInfo("Audio disabled: built without FMOD.");
        return {};
    }

    AudioEvent AudioSystem::Play(std::string_view event, const Float3& position)
    {
        LogInfo("Audio disabled: built without FMOD.");
        return {};
    }

    void AudioSystem::PlayOneShot(std::string_view event)
    {
        LogInfo("Audio disabled: built without FMOD.");
    }

    void AudioSystem::PlayOneShot(std::string_view event, const Float3& position)
    {
        LogInfo("Audio disabled: built without FMOD.");
    }

    void AudioSystem::PlayOneShotAttached(std::string_view event, Entity entity, float maxDopplerSpeed)
    {
        LogInfo("Audio disabled: built without FMOD.");
    }

    void AudioSystem::SetPlaying(bool playing)
    {
    }

    void AudioSystem::StopAll(EStopMode mode)
    {
    }

    void AudioSystem::SetBusVolume(std::string_view bus, float volume)
    {
    }

    void AudioSystem::SetBusPaused(std::string_view bus, bool paused)
    {
    }

    void AudioSystem::SetMasterVolume(float volume)
    {
    }

    void AudioSystem::SetPaused(bool paused)
    {
    }

    void AudioSystem::SetGlobalParameter(std::string_view name, float value)
    {
    }

    float AudioSystem::GetGlobalParameter(std::string_view name) const
    {
        return 0.0f;
    }

    AudioEvent AudioSystem::StartSnapshot(std::string_view name)
    {
        return {};
    }

    void AudioSystem::ProcessPendingBankDestroys()
    {
    }

    bool AudioEvent::IsValid() const { return false; }
    bool AudioEvent::IsPlaying() const { return false; }

    void AudioEvent::Stop(EStopMode)
    {
    }

    void AudioEvent::SetPaused(bool)
    {
    }

    void AudioEvent::SetParameter(const char*, float)
    {
    }

    void AudioEvent::SetPosition(const Float3&, const Float3&)
    {
    }

    bool AudioSystem::IsAlive(BankHandle) const { return false; }

    void AudioSystem::AcquireAsset(BankHandle)
    {
    }

    void AudioSystem::ReleaseAsset(BankHandle)
    {
    }
}
