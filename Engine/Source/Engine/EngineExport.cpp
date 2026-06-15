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
bool Engine_Initialize(void* hwnd, void* hinstance)
{
    RTGDEngine::NativeWindowHandle handle{};
#ifdef _WIN32
    handle.hwnd      = hwnd;
    handle.hinstance = hinstance;
#elif defined(__linux__)
    handle.display = hwnd;      // Display*
    handle.window  = reinterpret_cast<unsigned long>(hinstance);
#endif
    return false;
    //return RTGDEngine::Engine::Instance().Initialize(handle);
}

void Engine_Update(float deltaTime)
{
    Engine::Instance().Update(deltaTime);
}

// TODO: will be refactored in future
/*void Engine_HandleMessage(void* hwnd, unsigned int msg, uintptr_t wParam, intptr_t lParam)
{
    InputSystem::Instance().HandleMessage(
        static_cast<HWND>(hwnd), msg,
        static_cast<WPARAM>(wParam),
        static_cast<LPARAM>(lParam));
}*/

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
