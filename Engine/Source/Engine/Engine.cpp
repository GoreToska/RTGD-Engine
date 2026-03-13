#include <Windows.h>
#include <libloaderapi.h>

#include "Engine/Engine.h"
#include "Render/RenderSystem.h"
#include "Scene/RTGDEntityFactory.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    Engine& Engine::Instance()
    {
        static Engine instance;
        return instance;
    }

    bool Engine::Initialize(void* hwnd)
    {
        m_hwnd = hwnd;
        Logger::Instance().Initialize();

        RECT rect;
        GetClientRect(static_cast<HWND>(hwnd), &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        RTGDRenderSystem::Instance().Initialize(hwnd, width, height);

        LogInfo("Engine initialized with HWND: {}", m_hwnd);

        RTGDEntityFactory::CreateTriangle(m_world, RTGDRenderSystem::Instance().GetDevice(),
                                          RTGDRenderSystem::Instance().GetSwapChain(), "Shaders");

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

    void Engine::Update(float deltaTime)
    {
        if (m_gameModule)
            m_gameModule->Update(deltaTime);
    }

    void Engine::Render()
    {
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
}
