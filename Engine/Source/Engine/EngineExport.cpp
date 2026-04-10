//
// Created by gorev on 11.03.2026.
//

#include "Engine/EngineExport.h"

#include "Components/UUIDComponent.h"
#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "Render/RenderSystem.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "Tools/Logger.h"


using namespace RTGDEngine;

extern "C"
{
bool Engine_Initialize(void* hwnd)
{
    return Engine::Instance().Initialize(static_cast<HWND>(hwnd));
}

void Engine_Update(float deltaTime)
{
    Engine::Instance().Update(deltaTime);
}

void Engine_HandleMessage(void* hwnd, unsigned int msg, uintptr_t wParam, intptr_t lParam)
{
    InputSystem::Instance().HandleMessage(
        static_cast<HWND>(hwnd), msg,
        static_cast<WPARAM>(wParam),
        static_cast<LPARAM>(lParam));
}

void Engine_Resize(int w, int h)
{
    RTGDRenderSystem::Instance().Resize(w, h);
}

void Engine_Shutdown()
{
    Engine::Instance().Shutdown();
}

void Engine_GetEntities(EntityCallback callback)
{
    auto scene = SceneManager::Instance().GetActiveScene();
    if (!scene || !callback)
        return;

    auto& world = scene->GetWorld();

    scene->GetWorld().each([&](flecs::entity e, UUIDComponent go)
    {
        if (e.name().length() > 0)
        {
            LogInfo(e.name().c_str());
            callback(e.name().c_str(), e.id());
        }
    });
}
} // extern "C"
