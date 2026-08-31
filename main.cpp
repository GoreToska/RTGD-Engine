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

    if (!RTGDEngine::GEngine.Initialize(std::move(window))) {
        LogError("Failed to initialize engine!");
        return 1;
    }

    RTGDEngine::GEngine.LoadGameModule(GAME_MODULE_PATH);

    using Clock = std::chrono::high_resolution_clock;
    auto lastTime = Clock::now();

    while (RTGDEngine::GEngine.PollEvents()) {
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        RTGDEngine::GEngine.Update(dt);
    }

    RTGDEngine::GEngine.Shutdown();
    return 0;
}
