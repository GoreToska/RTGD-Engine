#pragma once

#include "Platform/WindowHandle.h"
#include <memory>
#include <string>
#include <flecs.h>

#include "Engine/IGameModule.h"
#include "Engine/IEngineInterface.h"
#include "Engine/EngineExport.h"
#include "Tools/RTGDMacros.h"


namespace RTGDEngine {
    class IPlatformWindow;

    class ENGINE_API Engine : public IEngineInterface {
        DECLARE_SINGLETON(Engine);

    public:
        // TODO: separate engine from window
        bool Initialize(IPlatformWindow* window);

        void Run();

        void Shutdown();

        bool LoadGameModule(const std::string &dllPath);

        void Update(float deltaTime);

        void Render();

        void CreateConsole();

    private:
        NativeWindowHandle m_window = {};

        std::unique_ptr<IGameModule> m_gameModule;

        //HMODULE m_gameDllHandle = nullptr;
        CreateGameModuleFunc m_createFunc = nullptr;
        DestroyGameModuleFunc m_destroyFunc = nullptr;

        void UpdateSystems(const flecs::world &world, float deltaTime);

        void PostUpdateSystems(const flecs::world &world, float deltaTime);
    };
}
