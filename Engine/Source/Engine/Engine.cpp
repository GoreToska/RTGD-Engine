#include <Windows.h>
#include <libloaderapi.h>

#include "Engine/Engine.h"
#include "Render/RenderSystem.h"
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

        RTGDRenderSystem::Instance().Initialize(hwnd, 100, 100);

        LogInfo("Engine initialized with HWND: {}", m_hwnd);

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
        if (m_dllHandle)
        {
            FreeLibrary(m_dllHandle);
            m_dllHandle = nullptr;
        }

        RTGDRenderSystem::Instance().Shutdown();
    }

    bool Engine::LoadGameModule(const std::string& dllPath)
    {
        m_dllHandle = LoadLibraryA(dllPath.c_str());
        if (!m_dllHandle)
        {
            LogError("Failed to load DLL: {}", dllPath);
            return false;
        }

        m_createFunc = reinterpret_cast<CreateGameModuleFunc>(GetProcAddress(m_dllHandle, "CreateGameModule"));
        m_destroyFunc = reinterpret_cast<DestroyGameModuleFunc>(GetProcAddress(m_dllHandle, "DestroyGameModule"));

        if (!m_createFunc || !m_destroyFunc)
        {
            LogError("Failed to get exported functions");
            FreeLibrary(m_dllHandle);
            m_dllHandle = nullptr;
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
        RTGDRenderSystem::Instance().EndFrame();

        /*if (m_gameModule)
            m_gameModule->Render();*/
    }
}
