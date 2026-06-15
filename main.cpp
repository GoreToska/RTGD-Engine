#include <iostream>

#include "Engine/Engine.h"
#include "Platform/PlatformFactory.h"

#include "Engine/Include/Render/RenderSystem.h"
#include "Input/InputSystem.h"
#include "Tools/Logger.h"

static bool g_running = true;


int main() {
    auto window = RTGDEngine::CreatePlatformWindow();

    if (!window->Create({"RTGD Engine", 1280, 720})) {
        LogError("Window Creation Failed!");
        return 1;
    }

    auto handle = window->GetHandle();
    if (!RTGDEngine::Engine::Instance().Initialize(window.get())) {
        LogError("Failed to initialize engine!");
        return 1;
    }

    using Clock = std::chrono::high_resolution_clock;
    auto lastTime = Clock::now();

    while (window->PollEvents()) {
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        RTGDEngine::Engine::Instance().Update(dt);
    }

    RTGDEngine::Engine::Instance().Shutdown();
    window->Destroy();
    return 0;
}
