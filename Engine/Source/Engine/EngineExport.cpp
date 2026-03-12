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
    return new Engine();
}

void Engine_Destroy(void* engine)
{
    delete static_cast<Engine*>(engine);
}

bool Engine_Initialize(void* engine, void* hwnd, int w, int h)
{
    return static_cast<Engine*>(engine)->Initialize(hwnd);
}

void Engine_Render(void* engine)
{
    auto* en = static_cast<Engine*>(engine);
    en->Instance().Render();
}

void Engine_Resize(void* engine, int w, int h)
{
    RTGDRenderSystem::Instance().Resize(w, h);
}

void Engine_Shutdown(void* engine)
{
    static_cast<RTGDRenderSystem*>(engine)->Shutdown();
}
} // extern "C"
