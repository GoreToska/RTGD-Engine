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

extern "C"
{
ENGINE_API void* Engine_Create();

ENGINE_API void Engine_Destroy(void* engine);

ENGINE_API bool Engine_Initialize(void* engine, void* hwnd, int w, int h);

ENGINE_API void Engine_Render(void* engine);

ENGINE_API void Engine_Resize(void* engine, int w, int h);

ENGINE_API void Engine_Shutdown(void* engine);
}
