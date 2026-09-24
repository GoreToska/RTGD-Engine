//
// Created by ivan on 9/22/26.
//

#include "Systems/AudioSystem.h"
#include "Tools/Logger.h"

#include <fmod_errors.h>
#include <fstream>

#include "AssetLoader/AssetManager.h"
#include "AssetLoader/PathResolve.h"
#include "Components/AudioSourceComponent.h"
#include "Components/TransformComponent.h"
#include "nlohmann/json.hpp"

namespace RTGDEngine
{
    static nlohmann::json LoadAudioConfigJson(const std::string& fullPath)
    {
        std::ifstream stream(fullPath);
        if (!stream)
        {
            LogError("Audio config file not found: '{}'. Fall back to defaults.", fullPath);
            return {};
        }

        try
        {
            nlohmann::json json;
            stream >> json;
            return json;
        }
        catch (const nlohmann::json::exception& e)
        {
            LogError("Audio config parse error '{}': {} - falling back to defaults", fullPath, e.what());
            return {};
        }
    }

    static std::string WithPrefix(std::string_view path, std::string_view prefix)
    {
        if (path.starts_with(prefix) || path.starts_with('{'))
            return std::string(path);
        if (path.starts_with('/'))
            path.remove_prefix(1);
        return std::string(prefix).append(path);
    }

    static FMOD_3D_ATTRIBUTES Make3DAttributes(const Float3& position)
    {
        FMOD_3D_ATTRIBUTES attr{};
        attr.position = {position.x, position.y, position.z};
        attr.forward = {0.0f, 0.0f, 1.0f};
        attr.up = {0.0f, 1.0f, 0.0f};
        return attr;
    }

    static FMOD_3D_ATTRIBUTES Make3DAttributes(const TransformComponent& transform)
    {
        FMOD_3D_ATTRIBUTES attr{};
        attr.position = {transform.Position.x, transform.Position.y, transform.Position.z};
        attr.forward = {transform.GetForward().x, transform.GetForward().y, transform.GetForward().z};
        attr.up = {transform.GetUp().x, transform.GetUp().y, transform.GetUp().z};
        return attr;
    }

    static FMOD_STUDIO_STOP_MODE ToFMODStopMode(EStopMode mode)
    {
        return mode == EStopMode::Immediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
    }

    bool AudioEvent::IsValid() const
    {
        return m_instance && m_instance->isValid();
    }

    bool AudioEvent::IsPlaying() const
    {
        FMOD_STUDIO_PLAYBACK_STATE state;
        return IsValid() && m_instance->getPlaybackState(&state) == FMOD_OK && state != FMOD_STUDIO_PLAYBACK_STOPPED;
    }

    void AudioEvent::Stop(EStopMode mode)
    {
        if (!IsValid())
            return;
        m_instance->stop(ToFMODStopMode(mode));
    }

    void AudioEvent::SetPaused(bool paused)
    {
        if (!IsValid())
            return;
        m_instance->setPaused(paused);
    }

    void AudioEvent::SetParameter(const char* name, float value)
    {
        if (!IsValid())
            return;
        FMOD_RESULT result = m_instance->setParameterByName(name, value);
        if (result != FMOD_OK)
            LogError("Sound SetParameter '{}' failed: {}", name, FMOD_ErrorString(result));
    }

    void AudioEvent::SetPosition(const Float3& position)
    {
        if (!IsValid())
            return;
        const FMOD_3D_ATTRIBUTES attributes = Make3DAttributes(position);
        m_instance->set3DAttributes(&attributes);
    }

    static FMOD_RESULT F_CALL FmodDebugCallback(FMOD_DEBUG_FLAGS flags, const char* file, int line,
                                                const char* func, const char* message)
    {
        if (flags & FMOD_DEBUG_LEVEL_ERROR)
            LogError("[FMOD] {}: {}", func, message);
        else if (flags & FMOD_DEBUG_LEVEL_WARNING)
            LogWarn("[FMOD] {}: {}", func, message);
        return FMOD_OK;
    }

    void AudioSystem::Initialize()
    {
        FMOD::Debug_Initialize(FMOD_DEBUG_LEVEL_WARNING, FMOD_DEBUG_MODE_CALLBACK, &FmodDebugCallback);

        FMOD_RESULT result = FMOD::Studio::System::create(&m_studioSystem);

        if (result != FMOD_OK)
        {
            LogError("Failed to initialize FMOD Studio system. Error: {}", FMOD_ErrorString(result));
            m_studioSystem = nullptr;
            return;
        }

        m_studioSystem->getCoreSystem(&m_coreSystem);
        m_coreSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_DEFAULT, 0);

        FMOD_STUDIO_INITFLAGS studioFlags = FMOD_STUDIO_INIT_NORMAL;
#ifdef RTGD_EDITOR
        studioFlags |= FMOD_STUDIO_INIT_LIVEUPDATE;
#endif
        result = m_studioSystem->initialize(1024, studioFlags, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK)
        {
            LogError("Failed to initialize studio system. Error: {}", FMOD_ErrorString(result));
            m_studioSystem->release();
            m_studioSystem = nullptr;
            m_coreSystem = nullptr;
            return;
        }

        nlohmann::json config = LoadAudioConfigJson(GetAbsolutePath("Assets/Config/Audio.json"));

        const std::filesystem::path banksDir = GetAbsolutePath(
            config.value("BanksDirectory", std::string("Assets/Audio/Desktop")));

        const auto bootstrap = config.value("Bootstrap",
                                            std::vector<std::string>{"Master.bank", "Master.strings.bank"});

        for (const std::string& name: bootstrap)
        {
            BankRef ref;
            ref.Resolve(GAssets().GetBank((banksDir / name).generic_string()));
            if (ref.IsResolved())
                m_bootstrapBanks.push_back(std::move(ref));
        }
    }

    void AudioSystem::Update(World& world, float deltaTime)
    {
        if (!m_studioSystem)
            return;

        ProcessPendingBankDestroys();

        world.each([&](AudioListenerComponent listener, const TransformComponent& t)
        {
            auto a = Make3DAttributes(t);
            m_studioSystem->setListenerAttributes(0, &a);
        });

        world.each([&](AudioSourceComponent& source, const TransformComponent& t)
        {
            if (!m_isPlaying)
            {
                source.Started = false;
                return;
            }

            if (source.PlayOnStart && !source.Started)
            {
                source.Play(t.Position);
                source.Started = true;
            }

            source.Playing.SetPosition(t.Position);
        });

        m_studioSystem->update();
    }

    void AudioSystem::Shutdown()
    {
        if (!m_studioSystem)
            return;

        m_bootstrapBanks.clear();
        ProcessPendingBankDestroys();

        m_studioSystem->unloadAll();
        m_studioSystem->release();
        m_studioSystem = nullptr;
        m_coreSystem = nullptr;
    }

    BankHandle AudioSystem::LoadBank(const std::string& absolutePath, uint64_t assetID)
    {
        if (!m_studioSystem)
            return INVALID_BANK_HANDLE;

        auto* bank = LoadBankFile(absolutePath);

        if (!bank)
        {
            return INVALID_BANK_HANDLE;
        }

        std::lock_guard lock(m_lifetimeMutex);

        uint32_t index;
        BankData data{bank, absolutePath};

        if (!m_banks.FreeList.empty())
        {
            index = m_banks.FreeList.back();
            m_banks.FreeList.pop_back();
            auto& slot = m_banks[index];
            slot.data = std::move(data);
            slot.refCount = 0;
            slot.pendingDestroy = 0;
            slot.assetID = assetID;
        }
        else
        {
            index = m_banks.size();
            m_banks.Slots.push_back({std::move(data), 0, 0, 0, assetID});
        }

        return BankHandle{index, m_banks[index].generation};
    }

    bool AudioSystem::IsAlive(BankHandle handle) const
    {
        return handle.Index() < m_banks.size() && m_banks[handle.Index()].generation == handle.Generation();
    }

    void AudioSystem::AcquireAsset(BankHandle handle)
    {
        std::lock_guard lock(m_lifetimeMutex);

        if (!IsAlive(handle))
            return;

        ++m_banks[handle.Index()].refCount;
    }

    void AudioSystem::ReleaseAsset(BankHandle handle)
    {
        std::lock_guard lock(m_lifetimeMutex);

        if (!IsAlive(handle))
            return;

        auto& slot = m_banks[handle.Index()];

        if (slot.refCount == 0)
            return;

        if (--slot.refCount == 0 && !slot.pendingDestroy)
        {
            slot.pendingDestroy = 1;
            m_banks.PendingDestroys.push_back(handle.Index());
        }
    }

    AudioEvent AudioSystem::Play(std::string_view event)
    {
        FMOD::Studio::EventInstance* instance = CreateInstance(event);
        if (!instance)
            return {};

        bool is3D = false;
        FMOD::Studio::EventDescription* desc = nullptr;
        instance->getDescription(&desc);
        desc->is3D(&is3D);
        if (is3D)
            LogWarn("3D sound event '{}' player with no position.", event);

        instance->start();
        instance->release();
        return AudioEvent{instance};
    }

    AudioEvent AudioSystem::Play(std::string_view event, const Float3& position)
    {
        FMOD::Studio::EventInstance* instance = CreateInstance(event);
        if (!instance)
            return {};

        const FMOD_3D_ATTRIBUTES attributes = Make3DAttributes(position);
        instance->set3DAttributes(&attributes);
        instance->start();
        instance->release();
        return AudioEvent{instance};
    }

    void AudioSystem::PlayOneShot(std::string_view event)
    {
        Play(event);
    }

    void AudioSystem::PlayOneShot(std::string_view event, const Float3& position)
    {
        Play(event, position);
    }

    void AudioSystem::SetPlaying(bool playing)
    {
        m_isPlaying = playing;

        if (!m_isPlaying)
            StopAll();
    }

    void AudioSystem::StopAll(EStopMode mode)
    {
        if (!m_studioSystem)
            return;

        FMOD::Studio::Bus* master = nullptr;
        if (m_studioSystem->getBus("bus:/", &master) == FMOD_OK)
            master->stopAllEvents(ToFMODStopMode(mode));

        for (auto& snapshot: m_activeSnapshots)
            snapshot.Stop(mode);

        m_activeSnapshots.clear();
    }

    void AudioSystem::SetBusVolume(std::string_view bus, float volume)
    {
        if (!m_studioSystem)
            return;

        FMOD::Studio::Bus* b = nullptr;
        auto result = m_studioSystem->getBus(WithPrefix(bus, "bus:/").c_str(), &b);
        if (result == FMOD_OK)
            b->setVolume(volume);
        else
            LogError("No such bus '{}': {}", bus, FMOD_ErrorString(result));
    }

    void AudioSystem::SetBusPaused(std::string_view bus, bool paused)
    {
        if (!m_studioSystem)
            return;

        FMOD::Studio::Bus* b = nullptr;
        auto result = m_studioSystem->getBus(WithPrefix(bus, "bus:/").c_str(), &b);
        if (result == FMOD_OK)
            b->setPaused(paused);
        else
            LogError("No such bus '{}': {}", bus, FMOD_ErrorString(result));
    }

    void AudioSystem::SetMasterVolume(float volume)
    {
        SetBusVolume("bus:/", volume);
    }

    void AudioSystem::SetPaused(bool paused)
    {
        SetBusPaused("bus:/", paused);
    }

    void AudioSystem::SetGlobalParameter(std::string_view name, float value)
    {
        if (!m_studioSystem)
            return;

        FMOD_RESULT result = m_studioSystem->setParameterByName(std::string(name).c_str(), value);
        if (result != FMOD_OK)
            LogError("Sound SetParameter '{}' failed: {}", name, FMOD_ErrorString(result));
    }

    float AudioSystem::GetGlobalParameter(std::string_view name) const
    {
        if (!m_studioSystem)
        {
            LogError("No studio system initialized!");
            return 0.0f;
        }

        float value = 0.0f;
        FMOD_RESULT result = m_studioSystem->getParameterByName(std::string(name).c_str(), &value);
        if (result != FMOD_OK)
            LogError("No such parameter on master bank: '{}'", name);

        return value;
    }

    AudioEvent AudioSystem::StartSnapshot(std::string_view name)
    {
        FMOD::Studio::EventInstance* instance = CreateInstance(name, "snapshot:/");
        if (!instance)
            return {};

        instance->start();
        instance->release();

        std::erase_if(m_activeSnapshots, [](const AudioEvent& event) { return !event.IsValid(); });
        AudioEvent snapshot{instance};
        m_activeSnapshots.push_back(snapshot);
        return snapshot;
    }

    FMOD::Studio::EventInstance* AudioSystem::CreateInstance(std::string_view event, std::string_view prefix)
    {
        if (!m_studioSystem)
            return nullptr;

        const std::string path = WithPrefix(event, prefix);

        FMOD::Studio::EventDescription* desc = nullptr;
        FMOD_RESULT result = m_studioSystem->getEvent(path.c_str(), &desc);
        if (result != FMOD_OK)
        {
            LogError("Sound event '{}' not found: {}. Is its bank loaded?", path, FMOD_ErrorString(result));
            return nullptr;
        }

        FMOD::Studio::EventInstance* inst = nullptr;
        result = desc->createInstance(&inst);
        if (result != FMOD_OK)
        {
            LogError("Failed to create sound instance of '{}': {}.", path, FMOD_ErrorString(result));
            return nullptr;
        }

        return inst;
    }

    void AudioSystem::ProcessPendingBankDestroys()
    {
        std::vector<uint32_t> destroyed; {
            std::lock_guard lock(m_lifetimeMutex);

            for (uint32_t i: m_banks.PendingDestroys)
            {
                auto& slot = m_banks[i];
                slot.pendingDestroy = 0;
                if (slot.refCount != 0)
                    continue;

                if (slot.data.bank)
                    slot.data.bank->unload();

                destroyed.push_back(BankHandle{i, slot.generation}.value);
                slot.data = {};
                ++slot.generation;
                m_banks.FreeList.push_back(i);
            }

            m_banks.PendingDestroys.clear();
        }

        for (uint32_t v: destroyed)
            if (OnBankDestroyed)
                OnBankDestroyed(v);
    }

    FMOD::Studio::Bank* AudioSystem::LoadBankFile(const std::string& absolutePath)
    {
        FMOD::Studio::Bank* bank = nullptr;
        FMOD_RESULT result = m_studioSystem->loadBankFile(absolutePath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
        if (result != FMOD_OK)
        {
            LogError("Failed to load bank '{}': {}", absolutePath, FMOD_ErrorString(result));
            return nullptr;
        }

        return bank;
    }
} // RTGDEngine
