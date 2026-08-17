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

typedef void (RTGD_CALLBACK*EntityCallback)(const char *name, uint64_t id, uint64_t parentID);

typedef void (RTGD_CALLBACK*EntityDestroyedCallback)(uint64_t id);

typedef void (RTGD_CALLBACK*EntityCreatedCallback)(const char *name, uint64_t id, uint64_t parentID);

typedef void (RTGD_CALLBACK*EntityRenamedCallback)(const char *newName, uint64_t id);

typedef void (RTGD_CALLBACK*EntityReparentedCallback)(uint64_t id, uint64_t oldParentID, uint64_t newParentID);

extern "C" {
ENGINE_API bool Engine_Initialize(void *nativeWindow, int width, int height);

ENGINE_API void Engine_InjectKey(int key, bool down);

ENGINE_API void Engine_InjectMouseButton(int button, bool down);

ENGINE_API void Engine_InjectMouseMove(float dx, float dy);

ENGINE_API void Engine_WarpCursorToCenter();

ENGINE_API void Engine_SetCursorVisible(bool visible);

ENGINE_API void Engine_Resize(int w, int h);

ENGINE_API void Engine_Shutdown();

ENGINE_API uint64_t Engine_PickEntity(int x, int y);

ENGINE_API void Engine_RenameEntity(uint64_t id, const char *name);

ENGINE_API void Engine_CreateEntity(const char *name);

ENGINE_API void Engine_DeleteEntity(uint64_t id);

ENGINE_API void Engine_ReparentEntity(uint64_t id, uint64_t parentID);

ENGINE_API void Engine_SetEntityCreatedCallback(EntityCreatedCallback cb);

ENGINE_API void Engine_SetEntityDestroyedCallback(EntityDestroyedCallback cb);

ENGINE_API void Engine_SetEntityRenamedCallback(EntityRenamedCallback cb);

ENGINE_API void Engine_SetEntityReparentedCallback(EntityReparentedCallback cb);

ENGINE_API uint32_t Editor_GetSelectedVersion(void);

ENGINE_API uint32_t Editor_GetHierarchyVersion(void);

ENGINE_API int Editor_GetSelectedEntityJson(char *buf, int cap);

ENGINE_API int Editor_GetHierarchyJson(char *buf, int cap);

ENGINE_API void Editor_SetSelectedEntity(uint64_t id);

ENGINE_API void Editor_SetFieldValue(uint64_t entityID, const char *component, const char *fieldPath,
                                     const char *value);
}
