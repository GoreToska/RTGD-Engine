//
// Created by ivan on 6/30/26.
//

#pragma once
#include "EventBus.h"

namespace RTGDEngine {
    class SceneManager;

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
            // TODO: EAssetType?
        };

        struct AssetUnloadedEvent {
            uint64_t ID;
            // TODO: EAssetType?
        };

        struct AssetReloadedEvent {
            uint64_t ID;
            // TODO: EAssetType?
        };

        struct OnAssetImportedEvent {
            uint64_t ID;
            // TODO: EAssetType?
        };

        struct SelectionChangedEvent {
            // This holds no info, selected entities should be accessed somewhere else
        };

        struct GizmoModeChangedEvent {
            // TODO: EGizmoMode?
        };

        inline constexpr EventKey<EntityCreatedEvent, SceneManager> OnEntityCreated{"entity.created"};
        inline constexpr EventKey<EntityDestroyedEvent, SceneManager> OnEntityDestroyed{"entity.destroyed"};
        inline constexpr EventKey<EntityReparentedEvent, SceneManager> OnEntityReparented{"entity.reparented"};
        inline constexpr EventKey<EntityRenamedEvent, SceneManager> OnEntityRenamed{"entity.renamed"};

        inline constexpr EventKey<SceneLoadedEvent, SceneManager> OnSceneLoaded{"scene.loaded"};
        inline constexpr EventKey<SceneUnloadedEvent, SceneManager> OnSceneUnloaded{"scene.unloaded"};
        inline constexpr EventKey<SceneCreatedEvent, SceneManager> OnSceneCreated{"scene.created"};
        inline constexpr EventKey<ActiveSceneChangedEvent, SceneManager> OnActiveSceneChanged{"scene.active_changed"};

        inline constexpr EventKey<WindowResizeEvent> OnWindowResized{"window.resized"};
        inline constexpr EventKey<WindowFocusEvent> OnWindowFocusChanged{"window.focus_changed"};
        inline constexpr EventKey<WindowClosedEvent> OnWindowClosed{"window.closed"};
        inline constexpr EventKey<WindowMinimizedEvent> OnWindowMinimized{"window.minimized"};
        inline constexpr EventKey<WindowMaximizedEvent> OnWindowMaximized{"window.maximized"};

        class AssetManager;
        inline constexpr EventKey<AssetLoadedEvent, AssetManager> OnAssetLoaded{"asset.loaded"};
        inline constexpr EventKey<AssetUnloadedEvent, AssetManager> OnAssetUnloaded{"asset.unloaded"};
        inline constexpr EventKey<AssetReloadedEvent, AssetManager> OnAssetReloaded{"asset.reloaded"};
        inline constexpr EventKey<OnAssetImportedEvent, AssetManager> OnAssetImported{"asset.imported"};

        inline constexpr EventKey<SelectionChangedEvent> OnSelectionChanged{"editor.selection_changed"};
        inline constexpr EventKey<GizmoModeChangedEvent> OnGizmoChanged{"editor.gizmo_changed"};
    }
}
