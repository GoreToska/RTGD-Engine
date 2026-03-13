//
// Created by gorev on 11.03.2026.
//

#include "Engine/EngineExport.h"

#include "Engine/Engine.h"
#include "Render/RenderSystem.h"


using namespace RTGDEngine;

extern "C"
{
void* Engine_Create()
{
    return &Engine::Instance();
}

bool Engine_Initialize(void* hwnd)
{
    return Engine::Instance().Initialize(hwnd);
}

void Engine_Render()
{
    Engine::Instance().Render();
}

void Engine_Resize(const int w, const int h)
{
    RTGDRenderSystem::Instance().Resize(w, h);
}

void Engine_Shutdown()
{
    Engine::Instance().Shutdown();
}
} // extern "C"
