//
// Created by gorev on 12.03.2026.
//

#pragma once
#include "Buffer.h"
#include "RefCntAutoPtr.hpp"
#include "AssetLoader/AssetManager.h"
#include "AssetLoader/Refs/AssetRef.h"
#include "Render/RenderHandle.h"

namespace RTGDEngine {
    struct MeshComponent {
        MeshRef Mesh;
        MaterialRef Material;

        MeshComponent() = default;

        MeshComponent(const MeshRef &meshRef, const MaterialRef &material)
            : Mesh(meshRef), Material(material) {
        }

        static void RegisterMeta(const flecs::world &world) {
            flecs::component<MeshComponent>(world, "MeshComponent")
                    .member<MeshRef>("Mesh")
                    .member<MaterialRef>("Material");

            world.observer<MeshComponent>()
                    .event(flecs::OnSet)
                    .each([](MeshComponent &m) {
                        if (!m.Mesh.Path.empty())
                            m.Mesh.Handle = AssetManager::Instance().GetMesh(m.Mesh.Path);
                    });
        }
    };
}
