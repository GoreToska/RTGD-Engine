#include "Engine/Engine.h"

#include <filesystem>
#include <chrono>

#include "AssetLoader/AssetManager.h"
#include "AssetLoader/PathResolve.h"
#include "Components/CameraComponent.h"
#include "Components/GameRootTag.h"
#include "Components/RigidbodyComponent.h"
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
#include "Systems/Physics/PhysicsSystem.h"

namespace RTGDEngine {
    constexpr uint32_t MAX_JOBS_TO_REMOVE = 32;

    void Engine::RegisterBaseSystems() {
        AddSystem([](flecs::world &, float dt) {
            GTimer.Update(dt);
        }, ESystemPhase::PreUpdate);

        AddSystem([&](flecs::world &world, float dt) {
            if (m_isPlayMode)
                GPhysics.Update(world, dt);
        }, ESystemPhase::FixedUpdate, 0);

        AddSystem([this](flecs::world &world, float dt) {
            if (!m_isPlayMode)
                EditorCameraSystem::Update(world, dt);
        }, ESystemPhase::Update, 0);

        AddSystem(MovementSystem::Update, ESystemPhase::Update, 0);
        AddSystem(CameraSystem::Update, ESystemPhase::Update, 20);

        AddSystem([](flecs::world &world, float) {
            LightSystem::Update(world);
        }, ESystemPhase::Update, 30);
    }

    bool Engine::Initialize(std::unique_ptr<IPlatformWindow> window) {
        m_platformWindow = std::move(window);

        GLogger.Initialize();

        GJobSystem.Initialize();

        GScene.Initialize();

        GPhysics.Initialize();

#ifdef RTGD_EDITOR
        GEditorBridge.Initialize();

        GScene.GetWorld().entity("EditorCamera").add<TransformComponent>().add<CameraComponent>().add<
            EditorCameraMovementComponent>().add<VelocityComponent>().add<UUIDComponent>();
#endif

        GRenderSystem.Initialize(m_platformWindow->GetHandle(), m_platformWindow->GetWidth(),
                                 m_platformWindow->GetHeight());

        GRenderResources.Initialize(GRenderSystem.GetDevice(),
                                    GRenderSystem.GetContext());

        GInput.AddWindowHandle(m_platformWindow.get());

        m_platformWindow->OnResize = [](int w, int h) { Instance().Resize(w, h); };
        m_platformWindow->OnClose = []() { Instance().OnClose(); };

#ifdef _WIN32
        LogInfo("Engine initialized with HWND: {}");
#elif defined(__linux__)
        LogInfo("Engine initialized with ID: {}", m_platformWindow->GetHandle().window);
#endif

        RegisterBaseSystems();

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

    void Engine::DestroyGameContent() {
        GScene.GetWorld().delete_with<GameRootTag>();
    }

    void Engine::UnloadGameModule() {
        if (m_gameLib && m_gameModule) {
            m_gameModule->Shutdown();
            ClearSystems(ESystemGroup::Game);
            DestroyGameContent();

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
        GEventBus.Process();
        UnloadGameModule();
        GRenderSystem.Shutdown();
        GPhysics.Shutdown();
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
        GJobSystem.Flush(MAX_JOBS_TO_REMOVE);
        GScene.ApplyPendingSceneChanges();
        GScene.ApplyPendingEntityCommands();
        GEventBus.Process();

        GInput.Update();

        if (GInput.IsDown(EInputAction::CtrlLeft) && GInput.IsPressed(
                EInputAction::ReloadGameModule)) {
            ReloadGameModule();
        }

        if (GInput.IsDown(EInputAction::CtrlLeft) && GInput.IsPressed(
                EInputAction::TogglePlayMode)) {
            TogglePlayMode();
        }

        auto &world = GScene.GetWorld();
        RunPhase(ESystemPhase::PreUpdate, world, deltaTime);

        m_fixedAccumulator += deltaTime;
        while (m_fixedAccumulator >= m_fixedTimeStep) {
            RunPhase(ESystemPhase::FixedUpdate, world, m_fixedTimeStep);
            m_fixedAccumulator -= m_fixedTimeStep;
        }

        RunPhase(ESystemPhase::Update, world, deltaTime);
        RunPhase(ESystemPhase::PostUpdate, world, deltaTime);

        GInput.PostUpdate();

#ifdef RTGD_EDITOR
        GEditorBridge.PublishSnapshot();
#endif

        Render();
    }

    void Engine::Render() {
        GRenderSystem.ApplyPendingResize(GScene.GetWorld());

        auto &rs = GRenderSystem;
        auto &device = rs.GetDevice();
        auto &context = rs.GetContext();
        auto &rm = GRenderResources;

        rm.FlushMeshUploads(device);
        rm.FlushTextureUploads(device, context);
        rm.ProcessPendingDestroys();

        GRenderSystem.ExecuteFrame(GScene.GetWorld());
        GRenderSystem.Present();
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

    void Engine::TogglePlayMode() {
        m_isPlayMode = !m_isPlayMode;

        if (m_isPlayMode) {
            LogInfo("Entering play mode.");
            m_gameModule->OnStart();
        } else {
            LogInfo("Exiting play mode.");
            m_gameModule->OnStop();
            ClearSystems(ESystemGroup::Game);
            DestroyGameContent();
            GScene.ReloadAll();
        }
    }

    void Engine::AddSystem(SystemFunc func, ESystemPhase phase, int order, ESystemGroup group) {
        auto &list = m_systems[static_cast<int>(phase)];
        list.push_back({std::move(func), order, group});
        std::stable_sort(list.begin(), list.end(), [](const SystemEntry &a, const SystemEntry &b) {
            return a.Order < b.Order;
        });
    }

    void Engine::ClearSystems(ESystemGroup group) {
        for (auto &list: m_systems) {
            std::erase_if(list, [group](const SystemEntry &entry) {
                return group == entry.Group;
            });
        }
    }

    void Engine::RunPhase(ESystemPhase phase, flecs::world &world, float deltaTime) {
        for (auto &entry: m_systems[static_cast<size_t>(phase)]) {
            entry.Func(world, deltaTime);
        }
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

        GRenderSystem.Resize(w, h);
        GInput.Resize(w, h);
        m_platformWindow->SetSize(w, h);
        GEventBus.Emit(Events::OnWindowResized, {w, h}, {});
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

        const flecs::entity e = GRenderSystem.PickEntity(x, y);
        {
            std::lock_guard<std::mutex> lock(m_pickMutex);
            m_pickRequest.Result = e.id();
            m_pickRequest.Done = true;
        }

        m_pickCV.notify_all();
    }
#endif

    void Engine::OnClose() {
        GEventBus.Emit(Events::OnWindowClosed, {}, {});
    }
}
