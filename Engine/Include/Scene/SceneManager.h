//
// Created by gorev on 23.03.2026.
//

#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "Engine/EngineExport.h"
#include <flecs.h>
#include <vector>

#include "Scene.h"
#include "Event/EventBus.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine {
    class Scene;
}

namespace RTGDEngine {
    namespace Events {
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

        inline constexpr EventKey<SceneLoadedEvent> OnSceneLoaded{"scene.loaded"};
        inline constexpr EventKey<SceneUnloadedEvent> OnSceneUnloaded{"scene.unloaded"};
        inline constexpr EventKey<SceneCreatedEvent> OnSceneCreated{"scene.created"};
        inline constexpr EventKey<ActiveSceneChangedEvent> OnActiveSceneChanged{"scene.active_changed"};
    }

    class ENGINE_API SceneManager {
        DECLARE_SINGLETON(SceneManager);

    public:
        struct PendingSceneLoad {
            std::string name;
            std::vector<Scene::EntityData> entities{};
        };

        void Initialize();

        std::shared_ptr<Scene> CreateScene(const std::string &name);

        void UnloadScene(const std::string &name);

        [[nodiscard]] std::shared_ptr<Scene> GetActiveScene() const;

        void SetActiveScene(const std::string &name);

        [[nodiscard]] bool HasScene(const std::string &name) const;

        std::shared_ptr<Scene> LoadSceneFromFile(const std::string &absolutePath);

        void RequestActiveScene(const std::string &name);

        void RequestUnloadScene(const std::string &name);

        template<typename Func>
        void Each(Func &&func) { m_world.each(std::forward<Func>(func)); }

        flecs::world &GetWorld();

        void RequestLoadScene(const std::string &absolutePath);

        void ApplyPendingSceneChanges();

    private:
        std::unordered_map<std::string, std::shared_ptr<Scene> > m_scenes{};
        std::shared_ptr<Scene> m_activeScene = nullptr;
        std::string m_pendingActive = {};
        std::vector<std::string> m_pendingUnloads = {};

        flecs::world m_world = flecs::world();

        std::mutex m_loadMutex = {};
        std::vector<PendingSceneLoad> m_completedLoads = {};
    };
} // RTGDEngine
