//
// Created by ivan on 8/12/26.
//

#include "Engine/EditorBridge.h"

#ifdef RTGD_EDITOR

#include <nlohmann/json.hpp>
#include <flecs.h>
#include <cstring>

#include "Components/UUIDComponent.h"
#include "Event/EventBus.h"
#include "Event/Events.h"
#include "Scene/SceneManager.h"


namespace RTGDEngine {
    namespace {
        nlohmann::json BuildNode(flecs::entity e) {
            nlohmann::json node;
            node["ID"] = e.id();
            node["Name"] = e.name().c_str();

            nlohmann::json children = nlohmann::json::array();
            e.children([&](flecs::entity child) {
                if (child.has<SceneEntity>())
                    children.push_back(BuildNode(child));
            });

            node["Children"] = std::move(children);
            return node;
        }

        int CopyOut(const std::string &src, char *buf, const int cap) {
            const int needed = static_cast<int>(src.size()) + 1;

            if (!buf || cap < needed)
                return needed;

            std::memcpy(buf, src.data(), needed);
            return needed;
        }

        void Bump(std::atomic<uint32_t> &version) {
            version.store(version.load(std::memory_order_relaxed) + 1, std::memory_order_release);
        }
    }


    void EditorBridge::Initialize() {
        auto &bus = EventBus::Instance();
        auto mark = [this](const auto &) { MarkHierarchyDirty(); };

        bus.Subscribe(Events::OnEntityCreated, mark);
        bus.Subscribe(Events::OnEntityDestroyed, mark);
        bus.Subscribe(Events::OnEntityRenamed, mark);
        bus.Subscribe(Events::OnEntityReparented, mark);
        bus.Subscribe(Events::OnActiveSceneChanged, mark);
        bus.Subscribe(Events::OnSceneUnloaded, mark);
    }

    void EditorBridge::PublishSnapshot() {
        const auto scene = SceneManager::Instance().GetActiveScene();
        if (!scene)
            return;

        if (m_hierarchyDirty.exchange(false)) {
            nlohmann::json root;
            root["Root"] = scene->GetRoot().id();

            nlohmann::json nodes = nlohmann::json::array();
            scene->GetRoot().children([&](flecs::entity child) {
                if (child.has<SceneEntity>())
                    nodes.push_back(BuildNode(child));
            });

            root["Nodes"] = std::move(nodes);

            std::string json = root.dump();
            std::lock_guard lock(m_mutex);
            if (json != m_hierarchyJson) {
                m_hierarchyJson = std::move(json);
                Bump(m_hierarchyVersion);
            }
        }

        std::string json = "{}";
        if (const uint64_t selected = m_selectedID.load(std::memory_order_relaxed)) {
            if (const flecs::entity e = SceneManager::Instance().GetEntity(selected); e.is_alive()) {
                static const flecs::entity_to_json_desc_t desc = [] {
                    flecs::entity_to_json_desc_t d = ECS_ENTITY_TO_JSON_INIT;
                    d.serialize_type_info = true;
                    return d;
                }();

                nlohmann::json obj = nlohmann::json::parse(e.to_json(&desc).c_str());
                obj["ID"] = e.id();
                obj["Name"] = e.name().c_str();
                json = obj.dump();
            }
        }

        std::lock_guard lock(m_mutex);
        if (json != m_selectedJson) {
            m_selectedJson = std::move(json);
            Bump(m_selectedVersion);
        }
    }

    void EditorBridge::MarkHierarchyDirty() {
        m_hierarchyDirty.store(true, std::memory_order_relaxed);
    }

    void EditorBridge::SetSelected(uint64_t id) {
        m_selectedID.store(id, std::memory_order_relaxed);
        EventBus::Instance().Emit(Events::OnSelectionChanged, {});
    }

    uint32_t EditorBridge::SelectedVersion() const {
        return m_selectedVersion.load(std::memory_order_acquire);
    }

    uint32_t EditorBridge::HierarchyVersion() const {
        return m_hierarchyVersion.load(std::memory_order_acquire);
    }

    int EditorBridge::CopySelectedJson(char *buf, int cap) const {
        std::lock_guard lock(m_mutex);
        return CopyOut(m_selectedJson, buf, cap);
    }

    int EditorBridge::CopyHierarchyJson(char *buf, int cap) const {
        std::lock_guard lock(m_mutex);
        return CopyOut(m_hierarchyJson, buf, cap);
    }
} // RTGDEngine

#endif
