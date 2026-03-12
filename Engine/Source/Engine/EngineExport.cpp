//
// Created by gorev on 11.03.2026.
//

#include "Engine/EngineExport.h"

#include "Render/RenderSystem.h"


using namespace RTGDEngine;

extern "C"
{
void* Engine_Create()
{
    return new RTGDRenderSystem();
}

void Engine_Destroy(void* engine)
{
    delete static_cast<RTGDRenderSystem*>(engine);
}

bool Engine_Initialize(void* engine, void* hwnd, int w, int h)
{
    return static_cast<RTGDRenderSystem*>(engine)->Initialize(hwnd, w, h);
}

void Engine_Render(void* engine)
{
    auto* rs = static_cast<RTGDRenderSystem*>(engine);
    rs->BeginFrame();
    rs->EndFrame();
}

void Engine_Resize(void* engine, int w, int h)
{
    static_cast<RTGDRenderSystem*>(engine)->Resize(w, h);
}

void Engine_Shutdown(void* engine)
{
    static_cast<RTGDRenderSystem*>(engine)->Shutdown();
}
} // extern "C"
