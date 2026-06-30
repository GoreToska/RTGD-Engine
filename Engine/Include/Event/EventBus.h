//
// Created by ivan on 6/30/26.
//

#pragma once

#include <functional>
#include <mutex>
#include <string_view>

#include "Engine/EngineExport.h"
#include "Tools/RTGDMacros.h"

// compile time id FNV-1a hash
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
namespace RTGDEngine {
    constexpr uint64_t EventID(std::string_view s) {
        uint64_t id = 1469598103934665603ull;
        for (const char c: s) {
            id ^= static_cast<uint8_t>(c);
            id *= 1099511628173ull;
        }

        return id;
    }

    constexpr uint64_t operator ""_hash(const char *str, size_t n) {
        return EventID({str, n});
    }

    template<typename Payload>
    struct EventKey {
        uint64_t ID;

        constexpr explicit EventKey(const uint64_t id) : ID(id) {
        }

        constexpr EventKey(const std::string_view name) : ID(EventID(name)) {
        }
    };

    using SubID = uint64_t;
    using OwnerID = uint64_t;

    class ENGINE_API EventBus {
        DECLARE_SINGLETON(EventBus);

    public:
        template<typename T, typename Fn>
        SubID Subscribe(EventKey<T> key, Fn &&callback, OwnerID owner = 0) {
            return SubscribeRaw(key.ID, [fn = std::move(callback)](const void *p, uint32_t) {
                fn(*static_cast<const T *>(p));
            }, owner);
        }

        template<typename P>
        void Emit(EventKey<P> key, const P &payload) {
            static_assert(std::is_trivially_copyable_v<P>,
                          "Event payload must be trivially copyable (POD): no std::string/vector/pointers");
            EmitRaw(key.ID, &payload, sizeof(P));
        }

        void EmitRaw(uint64_t id, const void *data, uint32_t size);

        SubID SubscribeRaw(uint64_t id, std::function<void(const void *, uint32_t)> fn,
                           OwnerID owner = 0);

        void Unsubscribe(SubID id);

        void UnsubscribeOwner(OwnerID owner);

        void Process();

    private:
        struct Sub {
            SubID ID;
            OwnerID owner;
            std::function<void(const void *, uint32_t)> callback;
        };

        struct QueuedEvent {
            uint64_t ID;
            std::vector<std::byte> data;
        };

        std::unordered_map<uint64_t, std::vector<Sub> > m_subs = {};
        std::unordered_map<SubID, uint64_t> m_subToEvent = {};

        std::mutex m_queueMutex = {};
        std::vector<QueuedEvent> m_queue = {};
        SubID m_nextSub = 1;
    };
}
