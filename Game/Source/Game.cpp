#include "pch.h"
#include "Game.h"

#include "GameExpoty.h"
#include "AssetLoader/PathResolve.h"
#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Components/VelocityComponent.h"
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


    /*RTGDEngine::SceneManager::Instance().GetActiveScene()->LoadFromFile(
        RTGDEngine::GetAbsolutePath("Assets/Scenes/Stress10k.scene"));*/
}

void Game::Update(float deltaTime) {
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

    m_player.set<RTGDEngine::VelocityComponent>({dir * m_speed, {}});
}

void Game::Shutdown() {
}

extern "C" {
GAME_API RTGDEngine::IGameModule *GetGameModule() {
    return &Game::Instance();
}
}
