//
// Created by gorev on 23.03.2026.
//

#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "Engine/EngineExport.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine
{
    class Scene;
}

namespace RTGDEngine
{
    class ENGINE_API SceneManager
    {
        DECLARE_SINGLETON(SceneManager);

    public:
        void Initialize();

        std::shared_ptr<Scene> CreateScene(const std::string& name);

        void UnloadScene(const std::string& name);

        [[nodiscard]] std::shared_ptr<Scene> GetActiveScene() const;

        void SetActiveScene(const std::string& name);

        [[nodiscard]] bool HasScene(const std::string& name) const;

    private:
        std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes{};
        std::shared_ptr<Scene> m_activeScene = nullptr;
    };
} // RTGDEngine
