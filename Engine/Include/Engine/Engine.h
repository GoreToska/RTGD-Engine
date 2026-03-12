#pragma once

#include <Windows.h>
#include <memory>
#include <string>

#include "Engine/IGameModule.h"
#include "Engine/IEngineInterface.h"
#include "Engine/EngineExport.h"


namespace RTGDEngine
{
    class ENGINE_API  Engine : public IEngineInterface
    {
    public:
        static Engine& Instance();

        bool Initialize(void* hwnd);

        void Run();

        void Shutdown();

        bool LoadGameModule(const std::string& dllPath);

        void Update(float deltaTime);

        void Render();

    private:
        void* m_hwnd = nullptr;
        std::unique_ptr<IGameModule> m_gameModule;

        HMODULE m_dllHandle = nullptr;
        CreateGameModuleFunc m_createFunc = nullptr;
        DestroyGameModuleFunc m_destroyFunc = nullptr;
    };
}
