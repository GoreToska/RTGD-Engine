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
                .each([](flecs::entity entity)
                {
                    EventBus::Instance().Emit(Events::OnEntityCreated, {entity.id()}, {});
                });

        m_world.observer<>().with<SceneEntity>().event(flecs::OnRemove)
                .each([](flecs::entity entity)
                {
                    EventBus::Instance().Emit(Events::OnEntityDestroyed, {entity.id()}, {});
                });

        m_world.observer<>().with<SceneEntity>().with<flecs::Identifier>(flecs::Name).event(flecs::OnSet)
                .each([](flecs::entity entity)
                {
                    EventBus::Instance().Emit(Events::OnEntityRenamed, {entity.id()}, {});
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
        EventBus::Instance().Emit(Events::OnSceneCreated, {scene->GetRoot()}, {});

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

        EventBus::Instance().Emit(Events::OnSceneUnloaded, {it->second->GetRoot()}, {});

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
        EventBus::Instance().Emit(Events::OnActiveSceneChanged, {prev, m_activeScene->GetRoot()}, {});
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
            EventBus::Instance().Emit(Events::OnSceneLoaded, {scene->GetRoot()}, {});
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

    void SceneManager::EnqueueCreateEntity(std::string name)
    {

    }

    void SceneManager::EnqueueDestroyEntity(uint64_t id)
    {
    }

    void SceneManager::EnqueueRenameEntity(uint64_t id, std::string name)
    {
    }

    void SceneManager::EnqueueReparentEntity(uint64_t id, uint64_t parentId)
    {
    }

    flecs::entity SceneManager::CreateEntity(const std::string& name)
    {

    }

    void SceneManager::DestroyEntity(flecs::entity e)
    {
    }

    void SceneManager::RenameEntity(flecs::entity e, const std::string& name)
    {
    }

    flecs::world& SceneManager::GetWorld()
    {
        return m_world;
    }

    void SceneManager::RequestLoadScene(const std::string& absolutePath)
    {
        JobSystem::Instance().Submit([this, absolutePath]()
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
