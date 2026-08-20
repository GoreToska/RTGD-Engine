#pragma once

#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <flecs.h>
#include <future>
#include <thread>
#include <functional>

#include "Engine/IGameModule.h"
#include "Engine/IEngineInterface.h"
#include "Platform/IPlatformWindow.h"
#include "Engine/EngineExport.h"
#include "Tools/RTGDMacros.h"
#include "Platform/DynamicLibraryFactory.h"



namespace RTGDEngine {
    enum class ESystemPhase {
        PreUpdate,
        FixedUpdate,
        Update,
        PostUpdate,
    };

    enum class ESystemGroup {
        Engine,
        Game,
    };

    class IPlatformWindow;

    class ENGINE_API Engine : public IEngineInterface {
        DECLARE_SINGLETON(Engine);

    public:
        // TODO: separate engine from window
        bool Initialize(std::unique_ptr<IPlatformWindow> window);

        bool Start(std::unique_ptr<IPlatformWindow> window);

        void Stop();

#ifdef RTGD_EDITOR
        uint64_t RequestPick(int x, int y);
#endif

        void UnloadGameModule();

        bool ReloadGameModule();

        void Shutdown();

        bool LoadGameModule(const std::string &dllPath);

        bool PollEvents() const;

        void Update(float deltaTime);

        void Render();

        void CreateConsole();

        void Resize(int w, int h);

        void TogglePlayMode();

        using SystemFunc = std::function<void(flecs::world &, float)>;

        void AddSystem(SystemFunc func, ESystemPhase phase = ESystemPhase::Update, int order = 0,
                       ESystemGroup group = ESystemGroup::Engine);

        void ClearSystems(ESystemGroup group);

    private:
        struct PickRequest {
            int X = 0;
            int Y = 0;
            bool Pending = false;
            uint64_t Result = 0;
            bool Done = false;
        };

        struct SystemEntry {
            SystemFunc Func;
            int Order = 0;
            ESystemGroup Group = ESystemGroup::Engine;
        };

        void RunPhase(ESystemPhase phase, flecs::world &world, float deltaTime);

        void RenderThreadMain(std::unique_ptr<IPlatformWindow> window, std::promise<bool> initPromise);

        void ApplyPendingResize();

        void DestroyGameContent();

        void RegisterBaseSystems();

#ifdef RTGD_EDITOR
        void ServicePick();
#endif

        void OnClose();

        // TODO: Engine owns window for now, but need to refactor this in future
        std::unique_ptr<IPlatformWindow> m_platformWindow = nullptr;

        std::unique_ptr<IDynamicLibrary> m_gameLib = nullptr;
        std::unique_ptr<IGameModule> m_gameModule = nullptr;
        GetGameModuleFunc m_getGameModuleFunc = nullptr;

        bool m_isPlayMode = false;

        std::thread m_renderThread = {};
        std::atomic<bool> m_isRunning = {false};
        std::mutex m_pickMutex = {};
        std::condition_variable m_pickCV = {};
        PickRequest m_pickRequest = {};

        std::mutex m_resizeMutex = {};
        bool m_resizePending = false;
        int m_pendingW = 0;
        int m_pendingH = 0;

        std::array<std::vector<SystemEntry>, 4> m_systems = {};
        float m_fixedTimeStep = 1.0f / 60.0f;
        float m_fixedAccumulator = 0.0f;

        void UpdateSystems(flecs::world &world, float deltaTime);

        void PostUpdateSystems(flecs::world &world, float deltaTime);
    };
}
