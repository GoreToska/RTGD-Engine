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

        /*CameraComponent cam;
        cam.AspectRatio = static_cast<float>(m_platformWindow->GetWidth()) / static_cast<float>(m_platformWindow->
                              GetHeight());

        SceneManager::Instance().GetActiveScene()->CreateEntity("EditorCamera")
                .set(UUIDComponent{})
                .set(TransformComponent{{0.0f, 0.0f, -3.0f}})
                .set(cam)
                .set(EditorCameraMovementComponent{})
                .set(VelocityComponent{});


        SceneManager::Instance().GetActiveScene()->CreateEntity("Cube")
                .set(UUIDComponent{})
                .set(TransformComponent{{2.0f, 0.0f, 0.0f}})
                .set(MeshComponent{
                    MeshRef{GetAbsolutePath("Assets/BoxTextured.gltf")},
                    MaterialRef{GetAbsolutePath("Assets/Materials/Cube.mat")}
                })
                .set(RenderComponent{});

        auto entt = SceneManager::Instance().GetActiveScene()->CreateEntity("Helmet")
                .set(UUIDComponent{})
                .set(TransformComponent{{0.0f, 0.0f, 0.0f}, Quaternion::RotationFromAxisAngle({1, 0, 0}, 45.0f)})
                .set(RenderComponent{})
                .set(MeshComponent{
                    MeshRef{GetAbsolutePath("Assets/Helmet/DamagedHelmet.gltf")},
                    MaterialRef{GetAbsolutePath("Assets/Materials/Helmet.mat")}
                });

        entt.get_ref<TransformComponent>()->Rotation = {1, 1, 0, 1};

        SceneManager::Instance().GetActiveScene()->CreateEntity("Spheres")
                .set(UUIDComponent{})
                .set(TransformComponent{{0.0f, 5.0f, 0.0f}})
                .set(RenderComponent{}).set(MeshComponent{
                    {GetAbsolutePath("Assets/PBRTest/MetalRoughSpheres.gltf")},
                    {GetAbsolutePath("Assets/Materials/Spheres.mat")}
                });

        // Light
        SceneManager::Instance().GetActiveScene()->CreateEntity("Sun")
                .set(UUIDComponent{})
                .set(DirectionalLightComponent{
                    .Direction = {-0.5f, -1.0f, -0.3f},
                    .Color = {1.0f, 0.95f, 0.8f},
                    .Intensity = 3.0f
                });

        SceneManager::Instance().GetActiveScene()->CreateEntity("Ambient")
                .set(UUIDComponent{})
                .set(AmbientLightComponent{
                    .Color = {0.2f, 0.2f, 0.2f},
                    .Intensity = 0.05f
                });

        TimerSystem::Instance().SetTimer([]() {
            auto scene = SceneManager::Instance().GetActiveScene();
            auto saved = scene->Serialize();
            LogInfo("Scene saved to json: {}", saved);

            SceneManager::Instance().GetActiveScene()->SaveToFile(GetAbsolutePath("Assets/Scenes/Default.scene"));
        }, 5);*/

            SceneManager::Instance().GetActiveScene()->LoadFromFile(GetAbsolutePath("Assets/Scenes/Default.scene"));

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

        UpdateSystems(SceneManager::Instance().GetActiveScene()->GetWorld(), deltaTime);

        if (m_gameModule)
            m_gameModule->Update(deltaTime);

        PostUpdateSystems(SceneManager::Instance().GetActiveScene()->GetWorld(), deltaTime);

        Render();
    }

    void Engine::Render() {
        RTGDRenderSystem::Instance().ApplyPendingResize(SceneManager::Instance().GetActiveScene()->GetWorld());

        auto &rs = RTGDRenderSystem::Instance();
        auto &device = rs.GetDevice();
        auto &context = rs.GetContext();
        auto &rm = RenderResourceManager::Instance();

        rm.FlushMeshUploads(device);
        rm.FlushTextureUploads(device, context);
        rm.ProcessPendingDestroys();


        RTGDRenderSystem::Instance().SetActiveCameraCB(SceneManager::Instance().GetActiveScene()->GetWorld());
        RTGDRenderSystem::Instance().RenderGeometry(SceneManager::Instance().GetActiveScene()->GetWorld());
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
