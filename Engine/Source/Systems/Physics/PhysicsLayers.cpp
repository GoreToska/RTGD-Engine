//
// Created by ivan on 8/26/26.
//

#include <fstream>

#include <nlohmann/json.hpp>

#include "Systems/Physics/PhysicsLayer.h"
#include "Tools/Logger.h"

namespace RTGDEngine::Layers
{
    namespace
    {
        void EnablePair(JPH::ObjectLayerPairFilterTable& matrix, uint32_t a, uint32_t b)
        {
            matrix.EnableCollision(Encode(a, true), Encode(b, true));
            matrix.EnableCollision(Encode(a, true), Encode(b, false));
            matrix.EnableCollision(Encode(a, false), Encode(b, true));
        }

        int IndexOf(const std::vector<std::string>& names, const std::string& name)
        {
            auto it = std::ranges::find(names, name);
            return it == names.end() ? -1 : static_cast<int>(it - names.begin());
        }
    }

    LayerConfig LoadLayerConfig(const std::string& fullPath)
    {
        LayerConfig cfg;
        cfg.Names = {"Default"};

        std::ifstream f(fullPath);
        if (!f)
        {
            LogError("Layers config not found '{}', falling back to single Default layer", fullPath);
        }
        else
            try
            {
                nlohmann::json j;
                f >> j;
                std::vector<std::string> names = j.at("Layers").get<std::vector<std::string>>();

                if (names.empty())
                    throw nlohmann::json::other_error::create(501, "Layers list is empty", &j);

                if (names.size() > kMaxGameplayLayers)
                {
                    LogError("Layers config: {} layers exceeds max {}.", names.size(), kMaxGameplayLayers);
                    names.resize(kMaxGameplayLayers);
                }
                cfg.Names = std::move(names);
            }
            catch (const nlohmann::json::exception& e)
            {
                LogError("Layers config parse error '{}': {} — falling back to single Default layer", fullPath,
                         e.what());
            }

        uint32_t numObjectLayers = cfg.Names.size() * 2;
        cfg.Matrix = std::make_unique<JPH::ObjectLayerPairFilterTable>(numObjectLayers);

        std::ifstream f2(fullPath);
        if (f2)
        {
            try
            {
                nlohmann::json j;
                f2 >> j;
                if (j.contains("Collide"))
                {
                    for (auto& pair: j.at("Collide"))
                    {
                        int a = IndexOf(cfg.Names, pair.at(0).get<std::string>());
                        int b = IndexOf(cfg.Names, pair.at(1).get<std::string>());
                        if (a < 0 || b < 0)
                        {
                            LogError("Layers config: unknown layer in Collide pair, skipping. Pair: {}, {}.", a, b);
                            continue;
                        }

                        EnablePair(*cfg.Matrix, a, b);
                    }
                }
            }
            catch (const nlohmann::json::exception& e)
            {
                LogError("Layers config Collide parse error: {}", e.what());
            }
        }

        return cfg;
    }

    void LayerRegistry::Build(const std::string& fullPath)
    {
        LayerConfig cfg = LoadLayerConfig(fullPath);
        m_names = std::move(cfg.Names);
        m_objectLayerPairFilter = std::move(cfg.Matrix);

        uint32_t numObjectLayers = GetNumLayers();
        m_broadPhaseLayerInterface = std::make_unique<JPH::BroadPhaseLayerInterfaceTable>(
            numObjectLayers, BroadPhaseLayers::NUM_LAYERS);

        for (uint32_t i = 0; i < m_names.size(); ++i)
        {
            m_broadPhaseLayerInterface->MapObjectToBroadPhaseLayer(Encode(i, false), BroadPhaseLayers::NON_MOVING);
            m_broadPhaseLayerInterface->MapObjectToBroadPhaseLayer(Encode(i, true), BroadPhaseLayers::MOVING);
        }

        m_objectVsBroadPhaseLayerFilter = std::make_unique<JPH::ObjectVsBroadPhaseLayerFilterTable>(
            *m_broadPhaseLayerInterface, BroadPhaseLayers::NUM_LAYERS, *m_objectLayerPairFilter, numObjectLayers);
    }

    int LayerRegistry::GetIndex(std::string_view name) const
    {
        auto it = std::ranges::find(m_names, name);
        return it == m_names.end() ? -1 : static_cast<int>(it - m_names.begin());
    }

    std::string LayerRegistry::GetName(int index) const
    {
        return m_names[index];
    }
}
