//
// Created by ivan on 6/30/26.
//

#include "Event/EventBus.h"

#include <algorithm>
#include <cstring>

namespace RTGDEngine {
    void EventBus::EmitRaw(uint64_t id, const void *data, uint32_t size) {
        QueuedEvent event = {};
        event.ID = id;
        event.data.resize(size);
        if (size && data) {
            std::memcpy(event.data.data(), data, size);
        }

        std::lock_guard lock(m_queueMutex);
        m_queue.push_back(std::move(event));
    }

    SubID EventBus::SubscribeRaw(uint64_t id, std::function<void(const void *, uint32_t)> fn, OwnerID owner) {
        SubID sid = m_nextSub++;
        m_subs[id].push_back({sid, owner, std::move(fn)});
        m_subToEvent[sid] = id;
        return sid;
    }

    void EventBus::Unsubscribe(SubID id) {
        auto it = m_subToEvent.find(id);
        if (it == m_subToEvent.end()) return;

        auto &vec = m_subs[it->second];
        std::erase_if(vec, [id](const Sub &s) {
            return s.ID == id;
        });

        m_subToEvent.erase(it);
    }

    void EventBus::UnsubscribeOwner(OwnerID owner) {
        for (auto &[eid, vec]: m_subs) {
            std::erase_if(vec,
                          [&](const Sub &s) {
                              if (s.owner == owner) {
                                  m_subToEvent.erase(s.ID);
                                  return true;
                              }
                              return false;
                          });
        }
    }

    void EventBus::Process() {
        std::vector<QueuedEvent> local;
        {
            std::lock_guard lock(m_queueMutex);
            local.swap(m_queue);
        }

        for (auto &[ID, data]: local) {
            auto it = m_subs.find(ID);
            if (it == m_subs.end()) continue;

            auto subs = it->second;
            for (auto &s: subs)
                s.callback(data.data(), static_cast<uint32_t>(data.size()));
        }
    }
}
