#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <flecs.h>

#include "Engine/IGameModule.h"
#include "Engine/IEngineInterface.h"
#include "Engine/EngineExport.h"


namespace RTGDEngine
{
    class ENGINE_API Engine : public IEngineInterface
    {
    public:
        static Engine& Instance();

        bool Initialize(HWND hwnd);

        void Run();

        void Shutdown();

        bool LoadGameModule(const std::string& dllPath);

        void Update(float deltaTime);

        void Render();

        void CreateConsole();

    private:
        void* m_hwnd = nullptr;
        std::unique_ptr<IGameModule> m_gameModule;

        flecs::world m_world;
        HMODULE m_gameDllHandle = nullptr;
        CreateGameModuleFunc m_createFunc = nullptr;
        DestroyGameModuleFunc m_destroyFunc = nullptr;

        void UpdateSystems(const flecs::world& world, float deltaTime);
        void PostUpdateSystems(const flecs::world& world, float deltaTime);
    };
}
