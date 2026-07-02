//
// Created by ivan on 7/2/26.
//

#pragma once
#include <cstdint>
#include <vector>

template<typename T>
class ResourcePool {
public:
    struct Slot {
        T data = {};
        uint32_t generation = 0;
        uint32_t refCount = 0;
        uint8_t pendingDestroy = 0;
        uint64_t assetID = 0;
    };

    std::vector<Slot> Slots = {};
    std::vector<uint32_t> FreeList = {};
    std::vector<uint32_t> PendingDestroys = {};

    Slot &operator[](uint32_t i) {
        return Slots[i];
    }

    const Slot &operator[](uint32_t i) const {
        return Slots[i];
    }

    [[nodiscard]] uint32_t size() const { return static_cast<uint32_t>(Slots.size()); }
};
