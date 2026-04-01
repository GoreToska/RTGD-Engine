#include <Windows.h>
#include <libloaderapi.h>

#include "Engine/Engine.h"

#include <filesystem>

#include "AssetLoader/AssetLoader.h"
#include "Components/CameraComponent.h"
#include "Components/UUID.h"
#include "Components/LightComponent.h"
#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Components/VelocityComponent.h"
#include "Input/InputSystem.h"
#include "JobSystem/JobSystem.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/RenderSystem.h"
#include "Scene/RTGDEntityFactory.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "Systems/CameraSystem.h"
#include "Systems/EditorCameraSystem.h"
#include "Systems/LightSystem.h"
#include "Systems/MovementSystem.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    constexpr uint32_t MAX_JOBS_TO_REMOVE = 32;

    bool Engine::Initialize(HWND hwnd)
    {
        m_hwnd = hwnd;
        Logger::Instance().Initialize();

        JobSystem::Instance().Initialize();

        SceneManager::Instance().Initialize();

        RegisterReflectionTypes(SceneManager::Instance().GetActiveScene()->GetWorld());

        RECT rect;
        GetClientRect(hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        RTGDRenderSystem::Instance().Initialize(hwnd, width, height);
        RenderResourceManager::Instance().Initialize(RTGDRenderSystem::Instance().GetDevice(),
                                                     RTGDRenderSystem::Instance().GetContext());

        InputSystem::Instance().Initialize(hwnd, width, height);

        LogInfo("Engine initialized with HWND: {}", m_hwnd);

        CameraComponent cam;
        cam.AspectRatio = static_cast<float>(width) / static_cast<float>(height);

        SceneManager::Instance().GetActiveScene()->CreateEntity("EditorCamera")
                .set(UUID{})
                .set(TransformComponent{{0.0f, 0.0f, -3.0f}})
                .set(cam)
                .set(EditorCameraMovementComponent{})
                .set(VelocityComponent{});


        MaterialHandle meshMat = PipelineFactory::CreateMeshPipeline(
            RTGDRenderSystem::Instance().GetDevice(),
            RTGDRenderSystem::Instance().GetSwapChain(),
            "Shaders");

        MeshHandle meshHandle = AssetLoader::Instance().LoadMeshAsync("Assets/BoxTextured.gltf");

        AssetLoader::Instance().LoadTextureAsync(
            "Assets/CesiumLogoFlat.png",
            meshMat,
            ETextureSlot::Diffuse,
            true,
            [meshMat](TextureHandle t)
            {
                RenderResourceManager::Instance().QueueTextureBind(meshMat, t);
                LogInfo("Texture queued for binding → tex={} mat={}", t, meshMat);
            });

        SceneManager::Instance().GetActiveScene()->CreateEntity("Cube")
                .set(UUID{})
                .set(TransformComponent{{2.0f, 0.0f, 0.0f}})
                .set(MeshComponent{meshHandle, meshMat})
                .set(RenderComponent{});

        MaterialHandle helmetMat = PipelineFactory::CreateMeshPipeline(
            RTGDRenderSystem::Instance().GetDevice(),
            RTGDRenderSystem::Instance().GetSwapChain(),
            "Shaders");


        MeshHandle helmetMesh = AssetLoader::Instance()
                .LoadMeshAsync("Assets/Helmet/DamagedHelmet.gltf");

        AssetLoader::Instance().LoadTextureAsync(
            "Assets/Helmet/Default_albedo.jpg",
            helmetMat, ETextureSlot::Diffuse, true);

        AssetLoader::Instance().LoadTextureAsync(
            "Assets/Helmet/Default_normal.jpg",
            helmetMat, ETextureSlot::Normal, false);

        AssetLoader::Instance().LoadTextureAsync(
            "Assets/Helmet/Default_metalRoughness.jpg",
            helmetMat, ETextureSlot::MetallicRoughness, false);

        AssetLoader::Instance().LoadTextureAsync(
            "Assets/Helmet/Default_AO.jpg",
            helmetMat, ETextureSlot::AO, false);

        SceneManager::Instance().GetActiveScene()->CreateEntity("Helmet")
                .set(UUID{})
                .set(TransformComponent{{0.0f, 0.0f, 0.0f}, Quaternion::RotationFromAxisAngle({1, 0, 0}, 45.0f)})
                .set(RenderComponent{})
                .set(MeshComponent{helmetMesh, helmetMat});


        MaterialHandle spheresMat = PipelineFactory::CreateMeshPipeline(
            RTGDRenderSystem::Instance().GetDevice(),
            RTGDRenderSystem::Instance().GetSwapChain(),
            "Shaders");


        MeshHandle spheresMesh = AssetLoader::Instance()
                .LoadMeshAsync("Assets/PBRTest/MetalRoughSpheres.gltf");

        AssetLoader::Instance().LoadTextureAsync(
            "Assets/PBRTest/Spheres_BaseColor.png",
            spheresMat, ETextureSlot::Diffuse, true);

        AssetLoader::Instance().LoadTextureAsync(
            "Assets/PBRTest/Spheres_MetalRough.png",
            spheresMat, ETextureSlot::MetallicRoughness, true);

        SceneManager::Instance().GetActiveScene()->CreateEntity("Spheres")
                .set(UUID{})
                .set(TransformComponent{{0.0f, 5.0f, 0.0f}})
                .set(RenderComponent{}).set(MeshComponent{spheresMesh, spheresMat});

        // Light
        SceneManager::Instance().GetActiveScene()->CreateEntity("Sun")
                .set(UUID{})
                .set(DirectionalLightComponent{
                    .Direction = {-0.5f, -1.0f, -0.3f},
                    .Color = {1.0f, 0.95f, 0.8f},
                    .Intensity = 3.0f
                });

        SceneManager::Instance().GetActiveScene()->CreateEntity("Ambient")
                .set(UUID{})
                .set(AmbientLightComponent{
                    .Color = {0.2f, 0.2f, 0.2f},
                    .Intensity = 0.05f
                });

        /*m_world.entity("PointLight1")
                .set(TransformComponent{{2.0f, 2.0f, 0.0f}})
                .set(PointLightComponent{
                    .Color = {1.0f, 0.5f, 0.1f},
                    .Intensity = 5.0f,
                    .Radius = 10.0f
                });*/

        return true;
    }

    void Engine::Run()
    {
    }

    void Engine::Shutdown()
    {
        if (m_gameModule && m_destroyFunc)
        {
            m_destroyFunc(m_gameModule.release());
        }

        if (m_gameDllHandle)
        {
            FreeLibrary(m_gameDllHandle);
            m_gameDllHandle = nullptr;
        }

        RTGDRenderSystem::Instance().Shutdown();
    }

    bool Engine::LoadGameModule(const std::string& dllPath)
    {
        m_gameDllHandle = LoadLibraryA(dllPath.c_str());
        if (!m_gameDllHandle)
        {
            LogError("Failed to load DLL: {}", dllPath);
            return false;
        }

        m_createFunc = reinterpret_cast<CreateGameModuleFunc>(GetProcAddress(m_gameDllHandle, "CreateGameModule"));
        m_destroyFunc = reinterpret_cast<DestroyGameModuleFunc>(GetProcAddress(m_gameDllHandle, "DestroyGameModule"));

        if (!m_createFunc || !m_destroyFunc)
        {
            LogError("Failed to get exported functions");
            FreeLibrary(m_gameDllHandle);
            m_gameDllHandle = nullptr;
            return false;
        }

        m_gameModule.reset(m_createFunc());
        if (m_gameModule)
            m_gameModule->Initialize();

        return true;
    }

    void Engine::Update(const float deltaTime)
    {
        JobSystem::Instance().Flush(MAX_JOBS_TO_REMOVE);

        UpdateSystems(SceneManager::Instance().GetActiveScene()->GetWorld(), deltaTime);

        if (m_gameModule)
            m_gameModule->Update(deltaTime);

        PostUpdateSystems(SceneManager::Instance().GetActiveScene()->GetWorld(), deltaTime);

        Render();
    }

    void Engine::Render()
    {
        RTGDRenderSystem::Instance().ApplyPendingResize();

        auto& rs = RTGDRenderSystem::Instance();
        auto& device = rs.GetDevice();
        auto& context = rs.GetContext();
        auto& rm = RenderResourceManager::Instance();

        rm.FlushMeshUploads(device);
        rm.FlushTextureUploads(device, context);


        RTGDRenderSystem::Instance().SetActiveCameraCB(SceneManager::Instance().GetActiveScene()->GetWorld());
        RTGDRenderSystem::Instance().RenderGeometry(SceneManager::Instance().GetActiveScene()->GetWorld());
        RTGDRenderSystem::Instance().RenderLighting();
        RTGDRenderSystem::Instance().Present();

        /*if (m_gameModule)
            m_gameModule->Render();*/
    }

    void Engine::CreateConsole()
    {
        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
    }

    void Engine::UpdateSystems(const flecs::world& world, float deltaTime)
    {
        InputSystem::Instance().Update();
        CameraSystem::Update(world, deltaTime);
        EditorCameraSystem::Update(world, deltaTime);
        MovementSystem::Update(world, deltaTime);

        LightSystem::Update(world);
    }

    void Engine::PostUpdateSystems(const flecs::world& world, float deltaTime)
    {
        InputSystem::Instance().PostUpdate();
    }
}
