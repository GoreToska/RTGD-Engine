//
// Created by gorev on 11.03.2026.
//

#include "Engine/EngineExport.h"

#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "Render/RenderSystem.h"


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
} // extern "C"
