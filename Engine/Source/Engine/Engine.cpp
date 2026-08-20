#include "Engine/Engine.h"

#include <filesystem>
#include <chrono>

#include "AssetLoader/AssetManager.h"
#include "AssetLoader/PathResolve.h"
#include "Components/CameraComponent.h"
#include "Components/GameRootTag.h"
#include "Components/TransformComponent.h"
#include "Components/UUIDComponent.h"
#include "Components/VelocityComponent.h"
#include "Engine/EditorBridge.h"
#include "Input/InputSystem.h"
#include "JobSystem/JobSystem.h"
#include "Platform/IPlatformWindow.h"
#include "Render/RenderResourceManager.h"
#include "Render/RenderSystem.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "Systems/CameraSystem.h"
#include "Systems/EditorCameraSystem.h"
#include "Systems/LightSystem.h"
#include "Systems/MovementSystem.h"
#include "Systems/TimerSystem.h"
#include "Tools/Logger.h"
#include "Event/Events.h"
#include "Event/EventBus.h"

namespace RTGDEngine {
    constexpr uint32_t MAX_JOBS_TO_REMOVE = 32;

    bool Engine::Initialize(std::unique_ptr<IPlatformWindow> window) {
        m_platformWindow = std::move(window);

        Logger::Instance().Initialize();

        JobSystem::Instance().Initialize();

        SceneManager::Instance().Initialize();

#ifdef RTGD_EDITOR
        EditorBridge::Instance().Initialize();

        SceneManager::Instance().GetWorld().entity("EditorCamera").add<TransformComponent>().add<CameraComponent>().add<
            EditorCameraMovementComponent>().add<VelocityComponent>().add<UUIDComponent>();
#endif

        RTGDRenderSystem::Instance().Initialize(m_platformWindow->GetHandle(), m_platformWindow->GetWidth(),
                                                m_platformWindow->GetHeight());

        RenderResourceManager::Instance().Initialize(RTGDRenderSystem::Instance().GetDevice(),
                                                     RTGDRenderSystem::Instance().GetContext());

        InputSystem::Instance().AddWindowHandle(m_platformWindow.get());

        m_platformWindow->OnResize = [](int w, int h) { Instance().Resize(w, h); };
        m_platformWindow->OnClose = []() { Instance().OnClose(); };

#ifdef _WIN32
        LogInfo("Engine initialized with HWND: {}");
#elif defined(__linux__)
        LogInfo("Engine initialized with ID: {}", m_platformWindow->GetHandle().window);
#endif

        return true;
    }

    bool Engine::Start(std::unique_ptr<IPlatformWindow> window) {
        std::promise<bool> initPromise;
        auto initFuture = initPromise.get_future();
        m_isRunning = true;
        m_renderThread = std::thread(
            [this, w = std::move(window), promise = std::move(initPromise)]() mutable {
                RenderThreadMain(std::move(w), std::move(promise));
            });

        return initFuture.get();
    }

    void Engine::Stop() {
        m_isRunning = false;

        {
            std::lock_guard<std::mutex> lock(m_pickMutex);
            m_pickRequest.Result = 0;
            m_pickRequest.Done = true;
        }

        m_pickCV.notify_all();

        if (m_renderThread.joinable()) {
            m_renderThread.join();
        }
    }

#ifdef RTGD_EDITOR
    uint64_t Engine::RequestPick(int x, int y) {
        std::unique_lock<std::mutex> lock(m_pickMutex);
        if (!m_isRunning) return 0;

        m_pickRequest = {x, y, true, 0, false};
        m_pickCV.wait(lock, [this] { return m_pickRequest.Done; });
        return m_pickRequest.Result;
    }
#endif

    void Engine::UnloadGameModule() {
        if (m_gameLib && m_gameModule) {
            m_gameModule->Shutdown();

            SceneManager::Instance().GetWorld().query_builder<>().with<GameRootTag>().build().each([](flecs::entity e) {
                e.destruct();
            });

            m_gameModule.release();
            m_gameLib.reset();
        }
    }

    bool Engine::ReloadGameModule() {
        auto path = m_gameLib->GetPath();
        UnloadGameModule();
        return LoadGameModule(path);
    }

    void Engine::Shutdown() {
        EventBus::Instance().Process();

        RTGDRenderSystem::Instance().Shutdown();

        UnloadGameModule();
    }

    bool Engine::LoadGameModule(const std::string &dllPath) {
        m_gameLib = CreateDynamicLibrary();

        if (!m_gameLib->Load(dllPath)) {
            LogError("Failed to load game module: {}.", dllPath);
            m_gameLib.reset();
            return false;
        }

        m_getGameModuleFunc = reinterpret_cast<GetGameModuleFunc>(m_gameLib->GetSymbol("GetGameModule"));

        if (!m_getGameModuleFunc) {
            LogError("Failed to get game module symbol: {}.", dllPath);
            m_gameLib.reset();
            return false;
        }

        LogInfo("Loaded game module: {}.", dllPath);

        m_gameModule.reset(m_getGameModuleFunc());
        m_gameModule->Initialize();

        return true;
    }

    bool Engine::PollEvents() const {
        return m_platformWindow->PollEvents();
    }

    void Engine::Update(const float deltaTime) {
        JobSystem::Instance().Flush(MAX_JOBS_TO_REMOVE);
        SceneManager::Instance().ApplyPendingSceneChanges();
        SceneManager::Instance().ApplyPendingEntityCommands();
        EventBus::Instance().Process();

        UpdateSystems(SceneManager::Instance().GetWorld(), deltaTime);

        if (InputSystem::Instance().IsDown(EInputAction::CtrlLeft) && InputSystem::Instance().IsPressed(
                EInputAction::ReloadGameModule)) {
            ReloadGameModule();
        }

        if (m_gameModule) {
            m_gameModule->Update(deltaTime);
        }

        PostUpdateSystems(SceneManager::Instance().GetWorld(), deltaTime);

#ifdef RTGD_EDITOR
        EditorBridge::Instance().PublishSnapshot();
#endif

        Render();
    }

    void Engine::Render() {
        RTGDRenderSystem::Instance().ApplyPendingResize(SceneManager::Instance().GetWorld());

        auto &rs = RTGDRenderSystem::Instance();
        auto &device = rs.GetDevice();
        auto &context = rs.GetContext();
        auto &rm = RenderResourceManager::Instance();

        rm.FlushMeshUploads(device);
        rm.FlushTextureUploads(device, context);
        rm.ProcessPendingDestroys();

        RTGDRenderSystem::Instance().ExecuteFrame(SceneManager::Instance().GetWorld());
        RTGDRenderSystem::Instance().Present();
    }

    void Engine::CreateConsole() {
#ifdef _WIN32
        AllocConsole();
        FILE *f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
#endif
    }

    void Engine::Resize(int w, int h) {
        std::lock_guard lk(m_resizeMutex);
        m_resizePending = true;
        m_pendingW = w;
        m_pendingH = h;
    }

    void Engine::RenderThreadMain(std::unique_ptr<IPlatformWindow> window, std::promise<bool> initPromise) {
        using clock = std::chrono::steady_clock;

        const bool ok = Initialize(std::move(window));
        initPromise.set_value(ok);

        if (!ok) {
            m_isRunning = false;
            return;
        }

        auto prev = clock::now();
        while (m_isRunning.load(std::memory_order_relaxed)) {
            ApplyPendingResize();

            const auto now = clock::now();
            const float dt = std::chrono::duration<float>(now - prev).count();
            prev = now;

            if (PollEvents())
                Update(dt);

#ifdef RTGD_EDITOR
            ServicePick();
#endif
        }

        Shutdown();
    }

    void Engine::ApplyPendingResize() {
        int w;
        int h;
        {
            std::lock_guard<std::mutex> lock(m_resizeMutex);
            if (!m_resizePending) return;
            m_resizePending = false;
            w = m_pendingW;
            h = m_pendingH;
        }

        RTGDRenderSystem::Instance().Resize(w, h);
        InputSystem::Instance().Resize(w, h);
        m_platformWindow->SetSize(w, h);
        EventBus::Instance().Emit(Events::OnWindowResized, {w, h}, {});
    }

#ifdef RTGD_EDITOR
    void Engine::ServicePick() {
        int x;
        int y;

        {
            std::lock_guard<std::mutex> lock(m_pickMutex);
            if (!m_pickRequest.Pending) return;
            x = m_pickRequest.X;
            y = m_pickRequest.Y;
            m_pickRequest.Pending = false;
        }

        const flecs::entity e = RTGDRenderSystem::Instance().PickEntity(x, y);
        {
            std::lock_guard<std::mutex> lock(m_pickMutex);
            m_pickRequest.Result = e.id();
            m_pickRequest.Done = true;
        }

        m_pickCV.notify_all();
    }
#endif

    void Engine::OnClose() {
        EventBus::Instance().Emit(Events::OnWindowClosed, {}, {});
    }

    void Engine::UpdateSystems(const flecs::world &world, float deltaTime) {
        InputSystem::Instance().Update();
        TimerSystem::Instance().Update(deltaTime);
        EditorCameraSystem::Update(world, deltaTime);
        MovementSystem::Update(world, deltaTime);
        CameraSystem::Update(world, deltaTime);

        LightSystem::Update(world);
    }

    void Engine::PostUpdateSystems(const flecs::world &world, float deltaTime) {
        InputSystem::Instance().PostUpdate();
    }
}
