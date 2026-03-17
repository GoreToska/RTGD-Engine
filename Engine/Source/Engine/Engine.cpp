#include <Windows.h>
#include <libloaderapi.h>

#include "Engine/Engine.h"

#include <filesystem>

#include "AssetLoader/AssetLoader.h"
#include "Components/CameraComponent.h"
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
#include "Systems/CameraSystem.h"
#include "Systems/EditorCameraSystem.h"
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

        RECT rect;
        GetClientRect(hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        RTGDRenderSystem::Instance().Initialize(hwnd, width, height);
        InputSystem::Instance().Initialize(hwnd, width, height);

        LogInfo("Engine initialized with HWND: {}", m_hwnd);

        CameraComponent cam;
        cam.AspectRatio = static_cast<float>(width) / static_cast<float>(height);

        m_world.entity("EditorCamera")
                .set(TransformComponent{{0.0f, 0.0f, -3.0f}})
                .set(cam)
                .set(EditorCameraMovementComponent{})
                .set(VelocityComponent{});

        RTGDEntityFactory::CreateTriangle(m_world, RTGDRenderSystem::Instance().GetDevice(),
                                          RTGDRenderSystem::Instance().GetSwapChain(), "Shaders");

        MaterialHandle meshMaterial = PipelineFactory::CreateMeshPipeline(
            RTGDRenderSystem::Instance().GetDevice(),
            RTGDRenderSystem::Instance().GetSwapChain(),
            "Shaders");

        MeshHandle meshHandle = AssetLoader::Instance().LoadMeshAsync("Assets/Box.gltf");

        m_world.entity("TestMesh")
                .set(TransformComponent{{2.0f, 2.0f, 2.0f}})
                .set(MeshComponent{meshHandle, meshMaterial})
                .set(RenderComponent{});

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

        UpdateSystems(m_world, deltaTime);

        if (m_gameModule)
            m_gameModule->Update(deltaTime);

        PostUpdateSystems(m_world, deltaTime);

        Render();
    }

    void Engine::Render()
    {
        RenderResourceManager::Instance().FlushGPUUploads(RTGDRenderSystem::Instance().GetDevice());

        RTGDRenderSystem::Instance().BeginFrame();
        RTGDRenderSystem::Instance().RenderScene(m_world);
        RTGDRenderSystem::Instance().EndFrame();

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
    }

    void Engine::PostUpdateSystems(const flecs::world& world, float deltaTime)
    {
        InputSystem::Instance().PostUpdate();
    }
}
