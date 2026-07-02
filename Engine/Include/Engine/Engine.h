#pragma once

#include <memory>
#include <string>
#include <flecs.h>

#include "Engine/IGameModule.h"
#include "Engine/IEngineInterface.h"
#include "Platform/IPlatformWindow.h"
#include "Engine/EngineExport.h"
#include "Tools/RTGDMacros.h"


namespace RTGDEngine {
    class IPlatformWindow;

    class ENGINE_API Engine : public IEngineInterface {
        DECLARE_SINGLETON(Engine);

    public:
        // TODO: separate engine from window
        bool Initialize(std::unique_ptr<IPlatformWindow> window);

        void Shutdown();

        bool LoadGameModule(const std::string &dllPath);

        bool PollEvents() const;

        void Update(float deltaTime);

        void Render();

        void CreateConsole();

        void Resize(int w, int h) const;


    private:
        void OnClose();

        // TODO: Engine owns window for now, but need to refactor this in future
        std::unique_ptr<IPlatformWindow> m_platformWindow = nullptr;
        std::unique_ptr<IGameModule> m_gameModule = nullptr;

        //HMODULE m_gameDllHandle = nullptr;
        CreateGameModuleFunc m_createFunc = nullptr;
        DestroyGameModuleFunc m_destroyFunc = nullptr;

        void UpdateSystems(const flecs::world &world, float deltaTime);

        void PostUpdateSystems(const flecs::world &world, float deltaTime);
    };
}
