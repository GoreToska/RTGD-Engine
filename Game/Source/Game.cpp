#include "pch.h"
#include "Game.h"

#include "GameExpoty.h"
#include "AssetLoader/PathResolve.h"
#include "Components/CameraComponent.h"
#include "Components/MeshComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Components/VelocityComponent.h"
#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "Scene/SceneManager.h"
#include "Tools/Logger.h"

Game &Game::Instance() {
    static Game instance;
    return instance;
}

void Game::Initialize() {
    LogInfo("Initializing game.");
    GScene.GetActiveScene()->LoadFromFile(
        GetAbsolutePath("Assets/Scenes/Default.scene"));


    /*RTGDEngine::GScene.GetActiveScene()->LoadFromFile(
        RTGDEngine::GetAbsolutePath("Assets/Scenes/Stress10k.scene"));*/
}

void Game::Update(float deltaTime) {
}

void Game::Shutdown() {
}

void Game::OnStart() {
    m_moveForward = GInput.RegisterAction("PlayerMoveForward");
    m_moveBackward = GInput.RegisterAction("PlayerMoveBackward");
    m_moveLeft = GInput.RegisterAction("PlayerMoveLeft");
    m_moveRight = GInput.RegisterAction("PlayerMoveRight");

    GInput.BindKey(m_moveForward, gainput::KeyW);
    GInput.BindKey(m_moveBackward, gainput::KeyS);
    GInput.BindKey(m_moveLeft, gainput::KeyA);
    GInput.BindKey(m_moveRight, gainput::KeyD);

    m_player = GScene.CreateEntity(
        "Player", GScene.GetGameRoot());

    m_player.set<MeshComponent>({{"Assets/BoxTextured.gltf"}, {"Assets/Materials/Cube.mat"}})
            .set<RenderComponent>({true, true})
            .set<TransformComponent>({{0.0f, 1.0f, 0.0f}})
            .set<VelocityComponent>({}).set<ColliderComponent>({})
            .set<RigidbodyComponent>({
                .AllowedDOFs = EPhysicsDOF::TranslationX | EPhysicsDOF::TranslationY | EPhysicsDOF::TranslationZ |
                               EPhysicsDOF::RotationY
            });

    m_playerCam = GScene.CreateEntity(
        "PlayerCamera", GScene.GetGameRoot());

    m_playerCam.set<CameraComponent>({.Priority = 1}).set<TransformComponent>({});

    GEngine.AddSystem(std::bind_front(&Game::PlayerMovementSystem, this), ESystemPhase::FixedUpdate, 0,
                      ESystemGroup::Game);
    GEngine.AddSystem(std::bind_front(&Game::CameraUpdate, this), ESystemPhase::Update, 10,
                      ESystemGroup::Game);

    auto fallingBox = GScene.GetActiveScene()->Find("Falling Box");
    if (auto phys = fallingBox.get_ref<RigidbodyComponent>()) {
        phys->OnCollisionEnter += [](const Events::CollisionEnterEvent &e) {
            LogInfo("[component] CollisionEnter: {} <-> {}", e.Target.name().c_str(), e.Other.name().c_str());
        };
        phys->OnCollisionStay += [](const Events::CollisionStayEvent &e) {
            LogInfo("[component] CollisionStay: {} <-> {}", e.Target.name().c_str(), e.Other.name().c_str());
        };
        phys->OnCollisionExit += [](const Events::CollisionExitEvent &e) {
            LogInfo("[component] CollisionExit: {} <-> {}", e.Target.name().c_str(), e.Other.name().c_str());
        };
        phys->OnTriggerEnter += [](const Events::TriggerEnterEvent &e) {
            LogInfo("[component] TriggerEnter: {} <-> {}", e.Target.name().c_str(), e.Other.name().c_str());
        };
        phys->OnTriggerStay += [](const Events::TriggerStayEvent &e) {
            LogInfo("[component] TriggerStay: {} <-> {}", e.Target.name().c_str(), e.Other.name().c_str());
        };
        phys->OnTriggerExit += [](const Events::TriggerExitEvent &e) {
            LogInfo("[component] TriggerExit: {} <-> {}", e.Target.name().c_str(), e.Other.name().c_str());
        };
    }


    if (auto rb = m_player.get_ref<RigidbodyComponent>()) {
        rb->OnCollisionEnter += [](const Events::CollisionEnterEvent &e) {
            LogInfo("[player] CollisionEnter: {} <-> {}", e.Target.name().c_str(), e.Other.name().c_str());
        };
    }
}

void Game::OnStop() {
}

void Game::PlayerMovementSystem(flecs::world &world, float deltaTime) {
    Float3 dir{};

    if (GInput.IsDown(m_moveForward)) {
        dir.z += 1;
    }

    if (GInput.IsDown(m_moveBackward)) {
        dir.z -= 1;
    }

    if (GInput.IsDown(m_moveRight)) {
        dir.x += 1;
    }

    if (GInput.IsDown(m_moveLeft)) {
        dir.x -= 1;
    }

    if (Diligent::length(dir) > 0.000001f)
        dir = Diligent::normalize(dir);


    auto &comp = m_player.get_mut<RigidbodyComponent>();
    comp.Velocity = Diligent::clamp(dir * m_speed + comp.Velocity, {-2.0f, -100.0f, -2.0f}, {2.0f, 100.0f, 2.0f});
}

void Game::CameraUpdate(flecs::world &world, float deltaTime) {
    auto &playerTransform = m_player.get_mut<TransformComponent>();
    auto &camTransform = m_playerCam.get_mut<TransformComponent>();

    camTransform.Position = playerTransform.Position + m_cameraOffset;
    camTransform.LookAt(playerTransform.Position);
}

extern "C" {
GAME_API IGameModule *GetGameModule() {
    return &GGame;
}
}
