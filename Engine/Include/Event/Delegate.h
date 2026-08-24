//
// Created by ivan on 8/24/26.
//

#pragma once

#include <functional>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace RTGDEngine {
    template<typename Owner>
    class DelegatesBadge {
        friend Owner;

        DelegatesBadge() = default;
    };

    template<typename Owner, typename... Args>
    class Delegate {
    public:
        using ID = uint64_t;

        ID Add(std::function<void(Args...)> func) {
            ID id = m_nextID++;
            m_subs.push_back({id, std::move(func)});
            return id;
        }

        void Remove(ID id) {
            std::erase_if(m_subs, [id](auto &sub) { return sub.id == id; });
        }

        void Broadcast(DelegatesBadge<Owner>, Args... args) const {
            for (auto &s: m_subs) {
                s.func(args...);
            }
        }

        ID operator+=(std::function<void(Args...)> func) {
            return Add(std::move(func));
        }

        void operator-=(ID id) {
            Remove(id);
        }

    private:
        struct Sub {
            ID id;
            std::function<void(Args...)> func;
        };

        std::vector<Sub> m_subs;
        ID m_nextID = 1;
    };
}
