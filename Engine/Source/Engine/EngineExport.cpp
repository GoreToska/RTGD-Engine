//
// Created by gorev on 11.03.2026.
//

#include "Engine/EngineExport.h"

#include <cstdint>

#include "Components/UUIDComponent.h"
#include "Engine/EditorBridge.h"
#include "Engine/Engine.h"
#include "Event/Events.h"
#include "Input/InputSystem.h"
#include "Render/RenderSystem.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "Tools/Logger.h"
#include "Platform/IPlatformWindow.h"
#include "Platform/Linux/EmbeddedLinuxWindow.h"
#include "Platform/PlatformFactory.h"


namespace RTGDEngine::Events {
    struct EntityCreatedEvent;
}

using namespace RTGDEngine;

extern "C" {
static EntityCreatedCallback g_onCreated = nullptr;
static EntityDestroyedCallback g_onDestroyed = nullptr;
static EntityRenamedCallback g_onRenamed = nullptr;
static EntityReparentedCallback g_onReparented = nullptr;

bool Engine_Initialize(void *nativeWindow, int width, int height) {
    GLogger.Initialize();

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

    // TODO: just for test, move somewhere this later
    GEventBus.Subscribe(Events::OnEntityCreated, [](const Events::EntityCreatedEvent &e) {
        if (!g_onCreated)
            return;
        auto ent = GScene.GetWorld().entity(e.entity);
        g_onCreated(ent.name().c_str(), e.entity, ent.parent().id());
    });

    GEventBus.Subscribe(Events::OnEntityDestroyed, [](const Events::EntityDestroyedEvent &e) {
        if (g_onDestroyed)
            g_onDestroyed(e.entity);
    });

    GEventBus.Subscribe(Events::OnEntityRenamed, [](const Events::EntityRenamedEvent &e) {
        if (!g_onRenamed)
            return;
        auto ent = GScene.GetWorld().entity(e.entity);
        g_onRenamed(ent.name().c_str(), e.entity);
    });

    GEventBus.Subscribe(Events::OnEntityReparented, [](const Events::EntityReparentedEvent &e) {
        if (g_onReparented)
            g_onReparented(e.entity, e.oldParent, e.newParent);
    });
    // --------------

    return GEngine.Start(std::move(platform));
}

void Engine_InjectKey(int key, bool down) {
    GInput.InjectKey(static_cast<gainput::Key>(key), down);
}

void Engine_InjectMouseButton(int button, bool down) {
    GInput.InjectMouseButton(static_cast<gainput::MouseButton>(button), down);
}

void Engine_InjectMouseMove(float dx, float dy) {
    GInput.InjectMouseMove(dx, dy);
}

void Engine_WarpCursorToCenter() {
    GInput.WarpCursorToCenter();
}

void Engine_SetCursorVisible(bool visible) {
    GInput.SetCursorVisible(visible);
}

void Engine_Resize(int w, int h) {
    GEngine.Resize(w, h);
}

void Engine_Shutdown() {
    GEngine.Stop();
}

uint64_t Engine_PickEntity(int x, int y) {
#ifdef RTGD_EDITOR
    if (x < 0 || y < 0)
        return 0;

    return GEngine.RequestPick(x, y);
#else
    return 0;
#endif
}

void Engine_RenameEntity(uint64_t id, const char *name) {
    GScene.EnqueueRenameEntity(id, name);
}

void Engine_CreateEntity(const char *name) {
    GScene.EnqueueCreateEntity(name);
}

void Engine_DeleteEntity(uint64_t id) {
    GScene.EnqueueDestroyEntity(id);
}

void Engine_ReparentEntity(uint64_t id, uint64_t parentID) {
    GScene.EnqueueReparentEntity(id, parentID);
}

void Engine_SetEntityCreatedCallback(EntityCreatedCallback cb) {
    g_onCreated = cb;
}

void Engine_SetEntityDestroyedCallback(EntityDestroyedCallback cb) {
    g_onDestroyed = cb;
}

void Engine_SetEntityRenamedCallback(EntityRenamedCallback cb) {
    g_onRenamed = cb;
}

void Engine_SetEntityReparentedCallback(EntityReparentedCallback cb) {
    g_onReparented = cb;
}

void Editor_SetFieldValue(uint64_t entityID, const char *component, const char *fieldPath, const char *value) {
    if (!component || !fieldPath || !value) return;

    GScene.EnqueueCommand(
        [entityID, comp = std::string(component), path = std::string(fieldPath), val = std::string(value)](
    flecs::world &world) {
            const flecs::entity e = world.entity(entityID);
            if (!e.is_alive()) return;

            const ecs_entity_t compId = ecs_lookup(world, comp.c_str());
            if (!compId || !e.has(compId)) return;

            void *ptr = ecs_get_mut_id(world, e, compId);
            if (!ptr) return;

            ecs_meta_cursor_t cur = ecs_meta_cursor(world, compId, ptr);
            if (!cur.valid) return;
            if (ecs_meta_push(&cur) != 0) return;
            if (ecs_meta_try_dotmember(&cur, path.c_str()) != 0) return;
            if (ecs_meta_set_string(&cur, val.c_str()) != 0) return;

            ecs_modified_id(world, e, compId);
        });
}

uint32_t Editor_GetSelectedVersion() {
#ifdef RTGD_EDITOR
    return GEditorBridge.SelectedVersion();
#endif
    return 0;
}

uint32_t Editor_GetHierarchyVersion() {
#ifdef RTGD_EDITOR
    return GEditorBridge.HierarchyVersion();
#endif
    return 0;
}

int Editor_GetSelectedEntityJson(char *buf, int cap) {
#ifdef RTGD_EDITOR
    return GEditorBridge.CopySelectedJson(buf, cap);
#endif
    return -1;
}

int Editor_GetHierarchyJson(char *buf, int cap) {
#ifdef RTGD_EDITOR
    return GEditorBridge.CopyHierarchyJson(buf, cap);
#endif
    return -1;
}

void Editor_SetSelectedEntity(uint64_t id) {
#ifdef RTGD_EDITOR
    GEditorBridge.SetSelected(id);
#endif
}
} // extern "C"
