#include "Engine/Engine.h"

#include <filesystem>

#include "AssetLoader/AssetManager.h"
#include "AssetLoader/PathResolve.h"
#include "Components/CameraComponent.h"
#include "Components/UUIDComponent.h"
#include "Components/LightComponent.h"
#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Components/VelocityComponent.h"
#include "Input/InputSystem.h"
#include "JobSystem/JobSystem.h"
#include "Platform/IPlatformWindow.h"
#include "Render/PipelineFactory.h"
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

namespace RTGDEngine {
    constexpr uint32_t MAX_JOBS_TO_REMOVE = 32;

    bool Engine::Initialize(std::unique_ptr<IPlatformWindow> window) {
        m_platformWindow = std::move(window);

        Logger::Instance().Initialize();

        JobSystem::Instance().Initialize();

        SceneManager::Instance().Initialize();

        RTGDRenderSystem::Instance().Initialize(m_platformWindow->GetHandle(), m_platformWindow->GetWidth(),
                                                m_platformWindow->GetHeight());

        RenderResourceManager::Instance().Initialize(RTGDRenderSystem::Instance().GetDevice(),
                                                     RTGDRenderSystem::Instance().GetContext());

        InputSystem::Instance().AddWindowHandle(m_platformWindow.get());

        m_platformWindow->OnResize = [](int w, int h) { Instance().Resize(w, h); };

#ifdef _WIN32
        LogInfo("Engine initialized with HWND: {}");
#elif defined(__linux__)
        LogInfo("Engine initialized with ID: {}", m_platformWindow->GetHandle().window);
#endif

        SceneManager::Instance().GetActiveScene()->LoadFromFile(GetAbsolutePath("Assets/Scenes/Default.scene"));
        SceneManager::Instance().GetActiveScene()->SaveToFile(GetAbsolutePath("Assets/Scenes/Default.scene"));

        TimerSystem::Instance().SetTimer([] {
            SceneManager::Instance().RequestLoadScene(GetAbsolutePath("Assets/Scenes/Additive.scene"));
            LogInfo("Timer: requested LOAD Additive");
        }, 3.0f);

        TimerSystem::Instance().SetTimer([] {
            SceneManager::Instance().RequestUnloadScene("Additive");
            LogInfo("Timer: requested UNLOAD Additive");
        }, 8.0f);

        return true;
    }

    void Engine::Shutdown() {
        if (m_gameModule && m_destroyFunc) {
            m_destroyFunc(m_gameModule.release());
        }

        // TODO: kill game dll
        /*if (m_gameDllHandle) {
            FreeLibrary(m_gameDllHandle);
            m_gameDllHandle = nullptr;
        }*/

        RTGDRenderSystem::Instance().Shutdown();
    }

    bool Engine::LoadGameModule(const std::string &dllPath) {
        // TODO: load game library
        /*m_gameDllHandle = LoadLibraryA(dllPath.c_str());
        if (!m_gameDllHandle) {
            LogError("Failed to load DLL: {}", dllPath);
            return false;
        }

        m_createFunc = reinterpret_cast<CreateGameModuleFunc>(GetProcAddress(m_gameDllHandle, "CreateGameModule"));
        m_destroyFunc = reinterpret_cast<DestroyGameModuleFunc>(GetProcAddress(m_gameDllHandle, "DestroyGameModule"));

        if (!m_createFunc || !m_destroyFunc) {
            LogError("Failed to get exported functions");
            FreeLibrary(m_gameDllHandle);
            m_gameDllHandle = nullptr;
            return false;
        }

        m_gameModule.reset(m_createFunc());
        if (m_gameModule)
            m_gameModule->Initialize();
            */

        return true;
    }

    bool Engine::PollEvents() const {
        return m_platformWindow->PollEvents();
    }

    void Engine::Update(const float deltaTime) {
        JobSystem::Instance().Flush(MAX_JOBS_TO_REMOVE);
        SceneManager::Instance().ApplyPendingSceneChanges();

        UpdateSystems(SceneManager::Instance().GetWorld(), deltaTime);

        if (m_gameModule)
            m_gameModule->Update(deltaTime);

        PostUpdateSystems(SceneManager::Instance().GetWorld(), deltaTime);

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


        RTGDRenderSystem::Instance().SetActiveCameraCB(SceneManager::Instance().GetWorld());
        RTGDRenderSystem::Instance().RenderGeometry(SceneManager::Instance().GetWorld());
        RTGDRenderSystem::Instance().RenderLighting();
        RTGDRenderSystem::Instance().Present();

        /*if (m_gameModule)
            m_gameModule->Render();*/
    }

    void Engine::CreateConsole() {
#ifdef _WIN32
        AllocConsole();
        FILE *f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
#endif
    }

    void Engine::Resize(int w, int h) const {
        RTGDRenderSystem::Instance().Resize(w, h);
        InputSystem::Instance().Resize(w, h);
        m_platformWindow->SetSize(w, h);
    }

    void Engine::UpdateSystems(const flecs::world &world, float deltaTime) {
        InputSystem::Instance().Update();
        TimerSystem::Instance().Update(deltaTime);
        CameraSystem::Update(world, deltaTime);
        EditorCameraSystem::Update(world, deltaTime);
        MovementSystem::Update(world, deltaTime);

        LightSystem::Update(world);
    }

    void Engine::PostUpdateSystems(const flecs::world &world, float deltaTime) {
        InputSystem::Instance().PostUpdate();
    }
}
