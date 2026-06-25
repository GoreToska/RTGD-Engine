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
    // Important!
    // Never write MeshRef & MaterialRef handles like Mesh.Handle = MyHandle
    // ECS-like setter should be used:
    //      entity.set(MeshComponent{ MeshRef{"MyModel.gltf"}, MyMaterial);
    // or in place:
    //      e.get_ref<MeshComponent>()->Mesh.Path = "MyModel.gltf";
    //      e.modified<MeshComponent>();   // this will emit OnSet

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
                    .each([](MeshComponent &mc) {
                        auto &am = AssetManager::Instance();
                        if (!mc.Mesh.Path.empty())
                            mc.Mesh.Handle = am.GetMesh(mc.Mesh.Path);
                    });
        }
    };
}
