//
// Created by ivan on 6/30/26.
//

#pragma once
#include "EventBus.h"
#include "Tools/Alias.h"

namespace RTGDEngine {
    class SceneManager;
    class Engine;
    class AssetManager;
    class RenderResourceManager;
    class PhysicsSystem;

    enum class EAssetType : uint8_t;


    namespace Events {
        struct EntityCreatedEvent {
            uint64_t entity;
        };

        struct EntityDestroyedEvent {
            uint64_t entity;
        };

        struct EntityReparentedEvent {
            uint64_t entity;
            uint64_t oldParent;
            uint64_t newParent;
        };

        struct EntityRenamedEvent {
            uint64_t entity;
        };

        struct SceneLoadedEvent {
            uint64_t sceneRoot;
        };

        struct SceneUnloadedEvent {
            uint64_t sceneRoot;
        };

        struct SceneCreatedEvent {
            uint64_t sceneRoot;
        };

        struct ActiveSceneChangedEvent {
            uint64_t previousRoot;
            uint64_t currentRoot;
        };

        struct WindowResizeEvent {
            int width;
            int height;
        };

        struct WindowFocusEvent {
            bool focus;
        };

        struct WindowClosedEvent {
            // TODO: exit code?
        };

        struct WindowMinimizedEvent {
        };

        struct WindowMaximizedEvent {
        };

        struct OnKeyEvent {
            // TODO: KeyID? State?
        };

        struct AssetLoadedEvent {
            uint64_t ID;
            EAssetType Type;
        };

        struct AssetUnloadedEvent {
            uint64_t ID;
            EAssetType Type;
        };

        struct AssetReloadedEvent {
            uint64_t ID;
            EAssetType Type;
        };

        struct OnAssetImportedEvent {
            uint64_t ID;
            EAssetType Type;
        };

        struct SelectionChangedEvent {
            // This holds no info, selected entities should be accessed somewhere else
        };

        struct GizmoModeChangedEvent {
            // TODO: EGizmoMode?
        };

        struct CollisionEnterEvent {
            Entity Target;
            Entity Other;
            Float3 Point;
            Float3 Normal;
        };

        struct CollisionStayEvent {
            Entity Target;
            Entity Other;
            Float3 Point;
            Float3 Normal;
        };

        struct CollisionExitEvent {
            Entity Target;
            Entity Other;
        };

        struct TriggerEnterEvent {
            Entity Target;
            Entity Other;
            Float3 Point;
            Float3 Normal;
        };

        struct TriggerStayEvent {
            Entity Target;
            Entity Other;
            Float3 Point;
            Float3 Normal;
        };

        struct TriggerExitEvent {
            Entity Target;
            Entity Other;
        };

        inline constexpr EventKey<EntityCreatedEvent, SceneManager> OnEntityCreated{"entity.created"};
        inline constexpr EventKey<EntityDestroyedEvent, SceneManager> OnEntityDestroyed{"entity.destroyed"};
        inline constexpr EventKey<EntityReparentedEvent, SceneManager> OnEntityReparented{"entity.reparented"};
        inline constexpr EventKey<EntityRenamedEvent, SceneManager> OnEntityRenamed{"entity.renamed"};

        inline constexpr EventKey<SceneLoadedEvent, SceneManager> OnSceneLoaded{"scene.loaded"};
        inline constexpr EventKey<SceneUnloadedEvent, SceneManager> OnSceneUnloaded{"scene.unloaded"};
        inline constexpr EventKey<SceneCreatedEvent, SceneManager> OnSceneCreated{"scene.created"};
        inline constexpr EventKey<ActiveSceneChangedEvent, SceneManager> OnActiveSceneChanged{"scene.active_changed"};

        inline constexpr EventKey<WindowResizeEvent, Engine> OnWindowResized{"window.resized"};
        inline constexpr EventKey<WindowFocusEvent, Engine> OnWindowFocusChanged{"window.focus_changed"};
        inline constexpr EventKey<WindowClosedEvent, Engine> OnWindowClosed{"window.closed"};
        inline constexpr EventKey<WindowMinimizedEvent, Engine> OnWindowMinimized{"window.minimized"};
        inline constexpr EventKey<WindowMaximizedEvent, Engine> OnWindowMaximized{"window.maximized"};

        inline constexpr EventKey<AssetLoadedEvent, RenderResourceManager> OnAssetLoaded{"asset.loaded"};
        inline constexpr EventKey<AssetUnloadedEvent, RenderResourceManager> OnAssetUnloaded{"asset.unloaded"};
        inline constexpr EventKey<AssetReloadedEvent, AssetManager> OnAssetReloaded{"asset.reloaded"};
        inline constexpr EventKey<OnAssetImportedEvent, AssetManager> OnAssetImported{"asset.imported"};

        inline constexpr EventKey<SelectionChangedEvent> OnSelectionChanged{"editor.selection_changed"};
        inline constexpr EventKey<GizmoModeChangedEvent> OnGizmoChanged{"editor.gizmo_changed"};

        inline constexpr EventKey<CollisionEnterEvent, PhysicsSystem> OnCollisionEnter{"physics.collision_enter"};
        inline constexpr EventKey<CollisionStayEvent, PhysicsSystem> OnCollisionStay{"physics.collision_stay"};
        inline constexpr EventKey<CollisionExitEvent, PhysicsSystem> OnCollisionExit{"physics.collision_exit"};
        inline constexpr EventKey<TriggerEnterEvent, PhysicsSystem> OnTriggerEnter{"physics.trigger_enter"};
        inline constexpr EventKey<TriggerStayEvent, PhysicsSystem> OnTriggerStay{"physics.trigger_stay"};
        inline constexpr EventKey<TriggerExitEvent, PhysicsSystem> OnTriggerExit{"physics.trigger_exit"};
    }
}
