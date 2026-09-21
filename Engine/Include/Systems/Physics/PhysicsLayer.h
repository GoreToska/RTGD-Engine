//
// Created by ivan on 8/21/26.
//

#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include "Jolt/Physics/Collision/ObjectLayerPairFilterTable.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h"
#include "Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h"

#include <nlohmann/json.hpp>

namespace RTGDEngine::BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr JPH::uint NUM_LAYERS(2);
}

namespace RTGDEngine::Layers {
    inline constexpr uint32_t kMaxGameplayLayers = 32;

    inline constexpr JPH::ObjectLayer Encode(uint32_t layerIndex, bool isMoving) {
        return static_cast<JPH::ObjectLayer>(layerIndex * 2 + (isMoving ? 1 : 0));
    }

    inline constexpr uint32_t DecodeGameplayLayer(JPH::ObjectLayer layer) {
        return layer / 2;
    }

    struct LayerConfig {
        std::vector<std::string> Names = {};
        std::unique_ptr<JPH::ObjectLayerPairFilterTable> Matrix;
    };

    LayerConfig LoadLayerConfig(const nlohmann::json& j);

    class LayerRegistry {
    public:
        void Build(const nlohmann::json& j);

        [[nodiscard]] const std::vector<std::string> &GetNames() const { return m_names; }

        [[nodiscard]] int GetIndex(std::string_view name) const;

        [[nodiscard]] std::string GetName(int index) const;

        [[nodiscard]] uint32_t GetNumLayers() const { return m_names.size() * 2; }

        [[nodiscard]] JPH::ObjectLayerPairFilterTable &GetPairFilter() const { return *m_objectLayerPairFilter; }

        [[nodiscard]] JPH::BroadPhaseLayerInterfaceTable &GetBroadPhaseLayerInterface() const {
            return *m_broadPhaseLayerInterface;
        }

        [[nodiscard]] JPH::ObjectVsBroadPhaseLayerFilterTable &GetObjectVsBroadPhaseLayerFilter() const {
            return *m_objectVsBroadPhaseLayerFilter;
        }

    private:
        std::vector<std::string> m_names = {};
        std::unique_ptr<JPH::ObjectLayerPairFilterTable> m_objectLayerPairFilter;
        std::unique_ptr<JPH::BroadPhaseLayerInterfaceTable> m_broadPhaseLayerInterface;
        std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterTable> m_objectVsBroadPhaseLayerFilter;
    };
}
