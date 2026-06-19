//
// Created by gorev on 11.03.2026.
//
#pragma once

#ifdef _WIN32
#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
#else
#define ENGINE_API __attribute__((visibility("default")))
#endif

#ifdef _WIN32
#define RTGD_CALLBACK __stdcall
#else
#define RTGD_CALLBACK
#endif

#include <cstdint>

typedef void (RTGD_CALLBACK*EntityCallback)(const char *name, uint64_t id);

extern "C" {
ENGINE_API bool Engine_Initialize(void *nativeWindow, int width, int height);

ENGINE_API void Engine_Update(float deltaTime);

ENGINE_API void Engine_HandleMessage(void *hwnd, unsigned int msg,
                                     uintptr_t wParam, intptr_t lParam);

ENGINE_API void Engine_Resize(int w, int h);

ENGINE_API void Engine_Shutdown();

ENGINE_API void Engine_GetEntities(EntityCallback callback);
}
