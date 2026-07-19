//
// Created by gorev on 11.03.2026.
//

#include "Engine/EngineExport.h"

#include "Components/UUIDComponent.h"
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

namespace RTGDEngine::Events
{
    struct EntityCreatedEvent;
}

using namespace RTGDEngine;

extern "C"
{
static EntityCreatedCallback g_onCreated = nullptr;
static EntityDestroyedCallback g_onDestroyed = nullptr;
static EntityRenamedCallback g_onRenamed = nullptr;
static EntityReparentedCallback g_onReparented = nullptr;

bool Engine_Initialize(void* nativeWindow, int width, int height)
{
    Logger::Instance().Initialize();

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
    EventBus::Instance().Subscribe(Events::OnEntityCreated, [](const Events::EntityCreatedEvent& e)
    {
        if (!g_onCreated)
            return;
        auto ent = SceneManager::Instance().GetWorld().entity(e.entity);
        g_onCreated(ent.name().c_str(), e.entity, ent.parent().id());
    });

    EventBus::Instance().Subscribe(Events::OnEntityDestroyed, [](const Events::EntityDestroyedEvent& e)
    {
        if (g_onDestroyed)
            g_onDestroyed(e.entity);
    });

    EventBus::Instance().Subscribe(Events::OnEntityRenamed, [](const Events::EntityRenamedEvent& e)
    {
        if (!g_onRenamed)
            return;
        auto ent = SceneManager::Instance().GetWorld().entity(e.entity);
        g_onRenamed(ent.name().c_str(), e.entity);
    });

    EventBus::Instance().Subscribe(Events::OnEntityReparented, [](const Events::EntityReparentedEvent& e)
    {
        if (g_onReparented)
            g_onReparented(e.entity, e.oldParent, e.newParent);
    });
    // --------------

    return Engine::Instance().Start(std::move(platform));
}

void Engine_InjectKey(int key, bool down)
{
    InputSystem::Instance().InjectKey(static_cast<gainput::Key>(key), down);
}

void Engine_InjectMouseButton(int button, bool down)
{
    InputSystem::Instance().InjectMouseButton(static_cast<gainput::MouseButton>(button), down);
}

void Engine_InjectMouseMove(float dx, float dy)
{
    InputSystem::Instance().InjectMouseMove(dx, dy);
}

void Engine_WarpCursorToCenter()
{
    InputSystem::Instance().WarpCursorToCenter();
}

void Engine_SetCursorVisible(bool visible)
{
    InputSystem::Instance().SetCursorVisible(visible);
}

void Engine_Resize(int w, int h)
{
    Engine::Instance().Resize(w, h);
}

void Engine_Shutdown()
{
    Engine::Instance().Stop();
}

void Engine_GetEntities(EntityCallback callback)
{
    auto scene = SceneManager::Instance().GetActiveScene();
    if (!scene || !callback)
        return;

    SceneManager::Instance().GetWorld().query_builder<UUIDComponent>()
            .with(flecs::ChildOf, scene->GetRoot())
            // only active scene? maybe need to have separate func to get all additive scenes
            .build()
            .each([&](flecs::entity e, UUIDComponent)
            {
                if (e.name().length() > 0)
                    callback(e.name().c_str(), e.id(), e.parent().id());
            });
}

uint64_t Engine_PickEntity(int x, int y)
{
#ifdef RTGD_EDITOR
    if (x < 0 || y < 0)
        return 0;

    return Engine::Instance().RequestPick(x, y);
#else
    return 0;
#endif
}

void Engine_RenameEntity(uint64_t id, const char* name)
{
    auto e = SceneManager::Instance().GetWorld().entity(id);
    if (!e.is_alive())
        return;
    e.set_name(name);
}

uint64_t Engine_CreateEntity(const char* name)
{
    auto e = SceneManager::Instance().GetActiveScene()->CreateEntity(name);
    if (!e.is_alive())
        return {};

    return e.id();
}

void Engine_DeleteEntity(uint64_t id)
{
    SceneManager::Instance().GetActiveScene()->DestroyEntity(SceneManager::Instance().GetWorld().entity(id));
}

void Engine_ReparentEntity(uint64_t id, uint64_t parentID)
{
    SceneManager::Instance().ReparentEntity(
        SceneManager::Instance().GetWorld().entity(id),
        SceneManager::Instance().GetWorld().entity(parentID));
}

void Engine_SetEntityCreatedCallback(EntityCreatedCallback cb)
{
    g_onCreated = cb;
}

void Engine_SetEntityDestroyedCallback(EntityDestroyedCallback cb)
{
    g_onDestroyed = cb;
}

void Engine_SetEntityRenamedCallback(EntityRenamedCallback cb)
{
    g_onRenamed = cb;
}
} // extern "C"
