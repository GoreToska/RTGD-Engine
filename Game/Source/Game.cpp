#include "pch.h"
#include "Game.h"

#include "GameExpoty.h"
#include "AssetLoader/PathResolve.h"
#include "Components/CameraComponent.h"
#include "Components/MeshComponent.h"
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
    RTGDEngine::SceneManager::Instance().GetActiveScene()->LoadFromFile(
        RTGDEngine::GetAbsolutePath("Assets/Scenes/Default.scene"));


    /*RTGDEngine::SceneManager::Instance().GetActiveScene()->LoadFromFile(
        RTGDEngine::GetAbsolutePath("Assets/Scenes/Stress10k.scene"));*/
}

void Game::Update(float deltaTime) {
}

void Game::Shutdown() {
}

void Game::OnStart() {
    auto &input = RTGDEngine::InputSystem::Instance();

    m_moveForward = input.RegisterAction("PlayerMoveForward");
    m_moveBackward = input.RegisterAction("PlayerMoveBackward");
    m_moveLeft = input.RegisterAction("PlayerMoveLeft");
    m_moveRight = input.RegisterAction("PlayerMoveRight");

    input.BindKey(m_moveForward, gainput::KeyW);
    input.BindKey(m_moveBackward, gainput::KeyS);
    input.BindKey(m_moveLeft, gainput::KeyA);
    input.BindKey(m_moveRight, gainput::KeyD);

    m_player = RTGDEngine::SceneManager::Instance().CreateEntity(
        "Player", RTGDEngine::SceneManager::Instance().GetGameRoot());

    m_player.set<RTGDEngine::MeshComponent>({{"Assets/BoxTextured.gltf"}, {"Assets/Materials/Cube.mat"}})
            .set<RTGDEngine::RenderComponent>({true, true})
            .set<RTGDEngine::TransformComponent>({{0.0f, 1.0f, 0.0f}})
            .set<RTGDEngine::VelocityComponent>({});


    m_playerCam = RTGDEngine::SceneManager::Instance().CreateEntity(
        "PlayerCamera", RTGDEngine::SceneManager::Instance().GetGameRoot());

    m_playerCam.set<RTGDEngine::CameraComponent>({.Priority = 1}).set<RTGDEngine::TransformComponent>({});

    auto &engine = RTGDEngine::Engine::Instance();
    engine.AddSystem(std::bind_front(&Game::PlayerMovementSystem, this), RTGDEngine::ESystemPhase::PreUpdate, 0,
                     RTGDEngine::ESystemGroup::Game);
    engine.AddSystem(std::bind_front(&Game::CameraUpdate, this), RTGDEngine::ESystemPhase::Update, 10,
                     RTGDEngine::ESystemGroup::Game);
}

void Game::OnStop() {
}

void Game::PlayerMovementSystem(flecs::world &world, float deltaTime) {
    auto &input = RTGDEngine::InputSystem::Instance();

    Float3 dir{};

    if (input.IsDown(m_moveForward)) {
        dir.z += 1;
    }

    if (input.IsDown(m_moveBackward)) {
        dir.z -= 1;
    }

    if (input.IsDown(m_moveRight)) {
        dir.x += 1;
    }

    if (input.IsDown(m_moveLeft)) {
        dir.x -= 1;
    }

    if (Diligent::length(dir) > 0.000001f)
        dir = Diligent::normalize(dir);

    m_player.set<RTGDEngine::VelocityComponent>({dir * m_speed, {}});
}

void Game::CameraUpdate(flecs::world &world, float deltaTime) {
    auto &playerTransform = m_player.get_mut<RTGDEngine::TransformComponent>();
    auto &camTransform = m_playerCam.get_mut<RTGDEngine::TransformComponent>();

    camTransform.Position = playerTransform.Position + m_cameraOffset;
    camTransform.LookAt(playerTransform.Position);
}

extern "C" {
GAME_API RTGDEngine::IGameModule *GetGameModule() {
    return &Game::Instance();
}
}
