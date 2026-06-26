//
// Created by gorev on 12.03.2026.
//

#pragma once
#include <cstdint>
#include <spdlog/spdlog.h>

namespace RTGDEngine {
    template<typename Tag>
    struct Handle {
        uint32_t value = UINT32_MAX;

        static constexpr uint32_t INDEX_BITS = 24;
        static constexpr uint32_t INDEX_MASK = (1 << INDEX_BITS) - 1;

        constexpr Handle() = default;

        constexpr Handle(uint32_t index, uint32_t generation) : value(
            (generation << INDEX_BITS) | (index & INDEX_MASK)) {
        }

        [[nodiscard]] uint32_t Index() const { return value & INDEX_MASK; }
        [[nodiscard]] uint32_t Generation() const { return value >> INDEX_BITS; }
        [[nodiscard]] bool IsValid() const { return value != UINT32_MAX; }

        bool operator==(const Handle &h) const = default;
    };

    struct MeshTag;
    struct MaterialTag;
    struct TextureTag;

    using MeshHandle = Handle<MeshTag>;
    using MaterialHandle = Handle<MaterialTag>;
    using TextureHandle = Handle<TextureTag>;

    constexpr MeshHandle INVALID_MESH_HANDLE = MeshHandle();;
    constexpr MaterialHandle INVALID_MATERIAL_HANDLE = MaterialHandle();
    constexpr TextureHandle INVALID_TEXTURE_HANDLE = TextureHandle();
}

template<typename Tag>
struct std::hash<RTGDEngine::Handle<Tag> > {
    size_t operator()(const RTGDEngine::Handle<Tag> &h) const {
        return h.value;
    };
};

template<typename Tag>
struct fmt::formatter<RTGDEngine::Handle<Tag> > {
    constexpr auto parse(fmt::format_parse_context &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const RTGDEngine::Handle<Tag> &h, FormatContext &ctx) const {
        if (!h.IsValid())
            return fmt::format_to(ctx.out(), "Handle(invalid)");
        return fmt::format_to(ctx.out(), "Handle(idx={}, gen={})",
                              h.Index(), h.Generation());
    }
};
