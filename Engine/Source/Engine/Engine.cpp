#include <Windows.h>
#include <iostream>
#include <libloaderapi.h>

#include "Engine/Engine.h"
#include "Components/EcsTest.h"

namespace Engine
{
    Engine& Engine::Instance()
    {
        static Engine instance;
        return instance;
    }

    bool Engine::Initialize(void* hwnd)
    {

        m_hwnd = hwnd;
        std::cout << "Engine initialized with HWND: " << hwnd << std::endl;

        Test();
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
    }

    bool Engine::LoadGameModule(const std::string& dllPath)
    {
        m_dllHandle = LoadLibraryA(dllPath.c_str());
        if (!m_dllHandle)
        {
            std::cerr << "Failed to load DLL: " << dllPath << std::endl;
            return false;
        }

        m_createFunc = (CreateGameModuleFunc)GetProcAddress(m_dllHandle, "CreateGameModule");
        m_destroyFunc = (DestroyGameModuleFunc)GetProcAddress(m_dllHandle, "DestroyGameModule");

        if (!m_createFunc || !m_destroyFunc)
        {
            std::cerr << "Failed to get exported functions" << std::endl;
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
        if (m_gameModule)
            m_gameModule->Render();
        // TODO: рендеринг через Filament
    }
}


extern "C"
{
    ENGINE_API bool Engine_Initialize(void* hwnd) {
        return Engine::Engine::Instance().Initialize(hwnd);
    }

    ENGINE_API void Engine_Run() {
        Engine::Engine::Instance().Run();
    }

    ENGINE_API void Engine_Shutdown() {
        Engine::Engine::Instance().Shutdown();
    }

    ENGINE_API bool Engine_LoadGameModule(const char* dllPath) {
        return Engine::Engine::Instance().LoadGameModule(dllPath);
    }

    ENGINE_API void Engine_Update(float deltaTime) {
        Engine::Engine::Instance().Update(deltaTime);
    }

    ENGINE_API void Engine_Render() {
        Engine::Engine::Instance().Render();
    }
}