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
#include "Platform/IPlatformWindow.h"
#include "Platform/Linux/EmbeddedLinuxWindow.h"
#include "Platform/PlatformFactory.h"

using namespace RTGDEngine;

extern "C" {
bool Engine_Initialize(void *nativeWindow, int width, int height) {
    Logger::Instance().Initialize();

    std::unique_ptr<IPlatformWindow> platform;
    NativeWindowHandle windowHandle = {};
#ifdef _WIN32
    windowHandle.hwnd = nativeWindow;
    windowHandle.hinstance = GetModuleHandle(nullptr);
    platform = CreateEmbeddedPlatformWindow(windowHandle);
#elif defined(__linux__)
    windowHandle.window = reinterpret_cast<unsigned long>(nativeWindow);
    windowHandle.width = width;
    windowHandle.height = height;
    platform = CreateEmbeddedPlatformWindow(windowHandle);
#endif

    return Engine::Instance().Initialize(std::move(platform));
}

void Engine_Update(float deltaTime) {
    if (Engine::Instance().PollEvents())
        Engine::Instance().Update(deltaTime);
}

void Engine_InjectKey(int key, bool down) {
    InputSystem::Instance().InjectKey(static_cast<gainput::Key>(key), down);
}

void Engine_InjectMouseButton(int button, bool down) {
    InputSystem::Instance().InjectMouseButton(static_cast<gainput::MouseButton>(button), down);
}

void Engine_InjectMousePosition(float normX, float normY) {
    InputSystem::Instance().InjectMousePosition(normX, normY);
}

void Engine_Resize(int w, int h) {
    Engine::Instance().Resize(w, h);
}

void Engine_Shutdown() {
    Engine::Instance().Shutdown();
}

void Engine_GetEntities(EntityCallback callback) {
    auto scene = SceneManager::Instance().GetActiveScene();
    if (!scene || !callback)
        return;

    SceneManager::Instance().GetWorld().query_builder<UUIDComponent>()
            .with(flecs::ChildOf, scene->GetRoot())
            // only active scene? maybe need to have separate func to get all additive scenes
            .build()
            .each([&](flecs::entity e, UUIDComponent) {
                if (e.name().length() > 0)
                    callback(e.name().c_str(), e.id(), e.parent().id());
            });
}
} // extern "C"
