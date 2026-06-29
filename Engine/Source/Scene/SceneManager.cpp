//
// Created by gorev on 23.03.2026.
//

#include "Scene/SceneManager.h"

#include "Scene/Scene.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    void SceneManager::Initialize()
    {
        CreateScene("Untitled");
    }

    std::shared_ptr<Scene> SceneManager::CreateScene(const std::string& name)
    {
        if (HasScene(name))
        {
            LogWarn("SceneManager: scene '{}' already exists", name);
            return m_scenes[name];
        }

        auto scene = std::make_shared<Scene>(name);
        m_scenes[name] = scene;

        if (!m_activeScene)
            m_activeScene = scene;

        LogInfo("SceneManager: created scene '{}'", name);
        return scene;
    }

    void SceneManager::UnloadScene(const std::string& name)
    {
        auto it = m_scenes.find(name);
        if (it == m_scenes.end())
            return;

        if (m_activeScene == it->second)
            m_activeScene = nullptr;

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

        m_activeScene = it->second;
        LogInfo("SceneManager: active scene → '{}'", name);
    }

    bool SceneManager::HasScene(const std::string& name) const
    {
        return m_scenes.contains(name);
    }
} // RTGDEngine
