//
// Created by ivan on 9/22/26.
//

#include "Systems/AudioSystem.h"
#include "Tools/Logger.h"

#include <fmod_errors.h>

namespace RTGDEngine
{
    void AudioSystem::Initialize()
    {
        FMOD::Studio::System::create(&m_studioSystem);
        m_studioSystem->getCoreSystem(&m_coreSystem);
        m_coreSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_DEFAULT, 0);

        FMOD_STUDIO_INITFLAGS studioFlags = FMOD_STUDIO_INIT_NORMAL;
#ifdef RTGD_EDITOR
        studioFlags |= FMOD_STUDIO_INIT_LIVEUPDATE;
#endif
        m_studioSystem->initialize(1024, studioFlags, FMOD_INIT_NORMAL, nullptr);
    }

    void AudioSystem::Update(World& world, float deltaTime)
    {
        ProcessPendingSoundDestroys();

        m_studioSystem->update();
    }

    void AudioSystem::Shutdown()
    {
        if (!m_studioSystem)
            return;

        m_studioSystem->unloadAll();
        m_studioSystem->release();
        m_studioSystem = nullptr;
        m_coreSystem = nullptr;
    }

    SoundHandle AudioSystem::LoadSound(const std::string& absolutePath, uint64_t assetID)
    {
        FMOD::Studio::Bank* bank = nullptr;
        FMOD_RESULT result = m_studioSystem->loadBankFile(absolutePath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);

        if (result != FMOD_OK)
        {
            LogError("Failed to load sound '{}': {}", absolutePath, FMOD_ErrorString(result));
            return INVALID_SOUND_HANDLE;
        }

        std::lock_guard lock(m_lifetimeMutex);

        uint32_t index;
        SoundData data{bank, absolutePath};

        if (!m_sounds.FreeList.empty())
        {
            index = m_sounds.FreeList.back();
            m_sounds.FreeList.pop_back();
            auto& slot = m_sounds[index];
            slot.data = std::move(data);
            slot.refCount = 0;
            slot.pendingDestroy = 0;
            slot.assetID = assetID;
        }
        else
        {
            index = m_sounds.size();
            m_sounds.Slots.push_back({std::move(data), 0, 0, 0, assetID});
        }

        return SoundHandle{index, m_sounds[index].generation};
    }

    bool AudioSystem::IsAlive(SoundHandle handle) const
    {
        return handle.Index() < m_sounds.size() && m_sounds[handle.Index()].generation == handle.Generation();
    }

    void AudioSystem::AcquireAsset(SoundHandle handle)
    {
        std::lock_guard lock(m_lifetimeMutex);

        if (!IsAlive(handle))
            return;

        ++m_sounds[handle.Index()].refCount;
    }

    void AudioSystem::ReleaseAsset(SoundHandle handle)
    {
        std::lock_guard lock(m_lifetimeMutex);

        if (!IsAlive(handle))
            return;

        auto& slot = m_sounds[handle.Index()];

        if (slot.refCount == 0)
            return;

        if (--slot.refCount == 0 && !slot.pendingDestroy)
        {
            slot.pendingDestroy = 1;
            m_sounds.PendingDestroys.push_back(handle.Index());
        }
    }

    void AudioSystem::ProcessPendingSoundDestroys()
    {
        std::vector<uint32_t> destroyed;
        std::lock_guard lock(m_lifetimeMutex);

        for (uint32_t i: m_sounds.PendingDestroys)
        {
            auto& slot = m_sounds[i];
            slot.pendingDestroy = 0;
            if (slot.refCount != 0)
                continue;

            if (slot.data.bank)
                slot.data.bank->unload();

            destroyed.push_back(SoundHandle{i, slot.generation}.value);
            slot.data = {};
            ++slot.generation;
            m_sounds.FreeList.push_back(i);
        }

        m_sounds.PendingDestroys.clear();

        for (uint32_t v: destroyed)
            if (OnSoundDestroyed)
                OnSoundDestroyed(v);
    }
} // RTGDEngine
