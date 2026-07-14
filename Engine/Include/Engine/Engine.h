#pragma once

#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <flecs.h>
#include <future>
#include <thread>

#include "Engine/IGameModule.h"
#include "Engine/IEngineInterface.h"
#include "Platform/IPlatformWindow.h"
#include "Engine/EngineExport.h"
#include "Tools/RTGDMacros.h"


namespace RTGDEngine {
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

        void Shutdown();

        bool LoadGameModule(const std::string &dllPath);

        bool PollEvents() const;

        void Update(float deltaTime);

        void Render();

        void CreateConsole();

        void Resize(int w, int h);

    private:
        struct PickRequest {
            int X = 0;
            int Y = 0;
            bool Pending = false;
            uint64_t Result = 0;
            bool Done = false;
        };

        void RenderThreadMain(std::unique_ptr<IPlatformWindow> window, std::promise<bool> initPromise);

        void ApplyPendingResize();

#ifdef RTGD_EDITOR
        void ServicePick();
#endif

        void OnClose();

        // TODO: Engine owns window for now, but need to refactor this in future
        std::unique_ptr<IPlatformWindow> m_platformWindow = nullptr;
        std::unique_ptr<IGameModule> m_gameModule = nullptr;

        //HMODULE m_gameDllHandle = nullptr;
        CreateGameModuleFunc m_createFunc = nullptr;
        DestroyGameModuleFunc m_destroyFunc = nullptr;

        std::thread m_renderThread = {};
        std::atomic<bool> m_isRunning = {false};
        std::mutex m_pickMutex = {};
        std::condition_variable m_pickCV = {};
        PickRequest m_pickRequest = {};

        std::mutex m_resizeMutex = {};
        bool m_resizePending = false;
        int m_pendingW = 0;
        int m_pendingH = 0;

        void UpdateSystems(const flecs::world &world, float deltaTime);

        void PostUpdateSystems(const flecs::world &world, float deltaTime);
    };
}
