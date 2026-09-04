//
// Created by gorev on 23.03.2026.
//

#include "Scene/SceneManager.h"

#include <filesystem>
#include <fstream>

#include "Components/UUIDComponent.h"
#include "Event/Events.h"
#include "JobSystem/JobSystem.h"
#include "Scene/Scene.h"
#include "Tools/Logger.h"
#include "Tools/MetaTypes.h"

namespace RTGDEngine
{
    void SceneManager::Initialize()
    {
        RegisterMetaTypes(m_world);

        m_world.observer<>().with<SceneEntity>().event(flecs::OnAdd)
                .each([](Entity entity)
                {
                    GEventBus().Emit(Events::OnEntityCreated, {entity.id()}, {});
                });

        m_world.observer<>().with<SceneEntity>().event(flecs::OnRemove)
                .each([](Entity entity)
                {
                    GEventBus().Emit(Events::OnEntityDestroyed, {entity.id()}, {});
                });

        m_world.observer<>().with<SceneEntity>().with<flecs::Identifier>(flecs::Name).event(flecs::OnSet)
                .each([](Entity entity)
                {
                    GEventBus().Emit(Events::OnEntityRenamed, {entity.id()}, {});
                });

        CreateScene("Untitled");
    }

    std::shared_ptr<Scene> SceneManager::CreateScene(const std::string& name)
    {
        if (HasScene(name))
        {
            LogWarn("SceneManager: scene '{}' already exists", name);
            return m_scenes[name];
        }

        auto scene = std::make_shared<Scene>(m_world, name);
        m_scenes[name] = scene;

        if (!m_activeScene)
            m_activeScene = scene;

        LogInfo("SceneManager: created scene '{}'", name);
        GEventBus().Emit(Events::OnSceneCreated, {scene->GetRoot()}, {});

        return scene;
    }

    void SceneManager::UnloadScene(const std::string& name)
    {
        auto it = m_scenes.find(name);
        if (it == m_scenes.end())
            return;
        if (it->second == m_activeScene)
        {
            LogWarn("SceneManager: cannot unload active scene '{}'", name);
            return;
        }

        GEventBus().Emit(Events::OnSceneUnloaded, {it->second->GetRoot()}, {});

        it->second->GetRoot().destruct();
        m_scenes.erase(it);
        LogInfo("SceneManager: unloaded scene '{}'", name);
    }

    std::shared_ptr<Scene> SceneManager::GetActiveScene() const
    {
        return m_activeScene;
    }

    void SceneManager::SetActiveScene(const std::string& name)
    {
        auto it = m_scenes.find(name);
        if (it == m_scenes.end())
        {
            LogError("SceneManager: scene '{}' not found", name);
            return;
        }

        uint64_t prev = m_activeScene ? m_activeScene->GetRoot() : 0;
        m_activeScene = it->second;
        GEventBus().Emit(Events::OnActiveSceneChanged, {prev, m_activeScene->GetRoot()}, {});
        LogInfo("SceneManager: active scene → '{}'", name);
    }

    bool SceneManager::HasScene(const std::string& name) const
    {
        return m_scenes.contains(name);
    }

    std::shared_ptr<Scene> SceneManager::LoadSceneFromFile(const std::string& absolutePath)
    {
        const std::string name = std::filesystem::path(absolutePath).stem().string();
        auto scene = CreateScene(name);
        scene->LoadFromFile(absolutePath);
        return scene;
    }

    void SceneManager::ReloadAll()
    {
        for (auto& [name, scene]: m_scenes)
        {
            scene->Reload();
        }
    }

    void SceneManager::RequestActiveScene(const std::string& name)
    {
        m_pendingActive = name;
    }

    void SceneManager::RequestUnloadScene(const std::string& name)
    {
        m_pendingUnloads.push_back(name);
    }

    void SceneManager::ApplyPendingSceneChanges()
    {
        std::vector<PendingSceneLoad> ready; {
            std::lock_guard lock(m_loadMutex);
            ready.swap(m_completedLoads);
        }

        for (auto& load: ready)
        {
            auto scene = CreateScene(load.name);
            scene->ApplyEntities(load.entities);
            GEventBus().Emit(Events::OnSceneLoaded, {scene->GetRoot()}, {});
        }

        if (!m_pendingActive.empty() && HasScene(m_pendingActive))
        {
            SetActiveScene(m_pendingActive);
            m_pendingActive.clear();
        }
        for (auto& n: m_pendingUnloads)
            UnloadScene(n);
        m_pendingUnloads.clear();
    }

    void SceneManager::EnqueueCreateEntity(std::string name, uint64_t parent)
    {
        std::lock_guard lock(m_commandsMutex);
        m_commands.emplace_back([this, n = std::move(name), parent](flecs::world& w)
        {
            CreateEntity(n, GetEntity(parent));
        });
    }

    void SceneManager::EnqueueDestroyEntity(uint64_t id)
    {
        std::lock_guard lock(m_commandsMutex);
        m_commands.emplace_back([this, id](flecs::world& w)
        {
            DestroyEntity(w.entity(id));
        });
    }

    void SceneManager::EnqueueRenameEntity(uint64_t id, std::string name)
    {
        std::lock_guard lock(m_commandsMutex);
        m_commands.emplace_back([this, id, n = std::move(name)](flecs::world& w)
        {
            RenameEntity(GetEntity(id), n);
        });
    }

    void SceneManager::EnqueueReparentEntity(uint64_t id, uint64_t parentId)
    {
        std::lock_guard lock(m_commandsMutex);
        m_commands.emplace_back([this, id, parentId](flecs::world& w)
        {
            ReparentEntity(GetEntity(id), GetEntity(parentId));
        });
    }

    void SceneManager::ApplyPendingEntityCommands()
    {
        std::vector<std::function<void(flecs::world&)>> commands{}; {
            std::lock_guard lock(m_commandsMutex);
            commands.swap(m_commands);
        }

        for (auto& command: commands)
        {
            command(m_world);
        }
    }

    Entity SceneManager::GetEntity(uint64_t id) const
    {
        return m_world.entity(id);
    }

    Entity SceneManager::Find(const std::string& name)
    {
        auto entity = m_activeScene->Find(name);
        if (entity)
            return entity;

        for (auto& [sceneName, scene]: m_scenes)
        {
            entity = scene->Find(name);
            if (entity)
                return entity;
        }

        return {};
    }

    Entity SceneManager::CreateEntity(const std::string& name, Entity parent)
    {
        if (parent == Entity::null())
            parent = m_activeScene->GetRoot();

        auto entity = m_world.entity(name.c_str()).child_of(parent)
                .add<UUIDComponent>()
                .add<SceneEntity>();

        LogInfo("Entity created '{}'", name);
        return entity;
    }

    Entity SceneManager::GetGameRoot()
    {
        return m_activeScene->GetOrCreateGameRoot();
    }

    void SceneManager::DestroyEntity(Entity e)
    {
        if (!e.is_alive())
            return;

        LogInfo("Entity destroyed '{}'", e.name().c_str());
        e.destruct();
    }

    void SceneManager::RenameEntity(Entity e, const std::string& name)
    {
        if (!e.is_alive())
            return;

        e.set_name(name.c_str());
        LogInfo("Rename entity '{}'", name.c_str());
    }

    void SceneManager::ReparentEntity(Entity e, Entity parent)
    {
        if (!e.is_alive())
            return;

        Entity oldParent = e.parent();
        Entity newParent = parent != Entity::null()
                               ? parent
                               : (m_activeScene ? m_activeScene->GetRoot() : Entity::null());
        if (newParent == 0)
            return;

        e.child_of(newParent);
        GEventBus().Emit(Events::OnEntityReparented, {e.id(), oldParent.id(), newParent.id()}, {});
    }

    void SceneManager::EnqueueCommand(std::function<void(flecs::world&)> func)
    {
        std::lock_guard lock(m_commandsMutex);
        m_commands.emplace_back(std::move(func));
    }

    void SceneManager::Shutdown()
    {
        for (auto& [name, scene]: m_scenes)
        {
            scene->Clear();
        }
    }

    flecs::world& SceneManager::GetWorld()
    {
        return m_world;
    }

    void SceneManager::RequestLoadScene(const std::string& absolutePath)
    {
        GJobSystem().Submit([this, absolutePath]()
        {
            std::ifstream f(absolutePath);
            if (!f)
            {
                LogError("Scene not found '{}'", absolutePath);
                return;
            }

            std::stringstream ss;
            ss << f.rdbuf();

            PendingSceneLoad load = {};
            load.name = std::filesystem::path(absolutePath).stem().string();
            try
            {
                load.entities = Scene::ParseScene(ss.str());
            }
            catch (const std::exception& e)
            {
                LogError("Failed to parse scene '{}': {} ", absolutePath, e.what());
                return;
            }

            std::lock_guard lock(m_loadMutex);
            m_completedLoads.push_back(std::move(load));
        });
    }
} // RTGDEngine
