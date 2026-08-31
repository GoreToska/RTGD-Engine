#include "pch.h"
#include "Game.h"

#include "GameExpoty.h"
#include "AssetLoader/PathResolve.h"
#include "Components/CameraComponent.h"
#include "Components/CharacterControllerComponent.h"
#include "Components/GroundCheckComponent.h"
#include "Components/MeshComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Components/VelocityComponent.h"
#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "Scene/SceneManager.h"
#include "Tools/Logger.h"

Game& Game::Instance()
{
    static Game instance;
    return instance;
}

void Game::Initialize()
{
    LogInfo("Initializing game.");
    GScene.GetActiveScene()->LoadFromFile(
        GetAbsolutePath("Assets/Scenes/Default.scene"));


    /*RTGDEngine::GScene.GetActiveScene()->LoadFromFile(
        RTGDEngine::GetAbsolutePath("Assets/Scenes/Stress10k.scene"));*/
}

void Game::Update(float deltaTime)
{
}

void Game::Shutdown()
{
}

void Game::SetupInput()
{
    m_moveForward = GInput.RegisterAction("PlayerMoveForward");
    m_moveBackward = GInput.RegisterAction("PlayerMoveBackward");
    m_moveLeft = GInput.RegisterAction("PlayerMoveLeft");
    m_moveRight = GInput.RegisterAction("PlayerMoveRight");
    m_interact = GInput.RegisterAction("Interact");
    m_jump = GInput.RegisterAction("Jump");

    GInput.BindKey(m_moveForward, gainput::KeyW);
    GInput.BindKey(m_moveBackward, gainput::KeyS);
    GInput.BindKey(m_moveLeft, gainput::KeyA);
    GInput.BindKey(m_moveRight, gainput::KeyD);
    GInput.BindKey(m_interact, gainput::KeyE);
    GInput.BindKey(m_jump, gainput::KeySpace);

    GInput.SetRelativeMouseMode(true);
}

void Game::OnStart()
{
    SetupInput();

    m_player = GScene.CreateEntity("Player", GScene.GetGameRoot());

    m_player.set<TransformComponent>({{0.0f, 1.0f, 0.0f}})
            .set<ColliderComponent>({
                .Shape = EPhysicsShape::Capsule, .Extents = {0.3f, 0.5f, 0.0}
            })
            .set<CharacterControllerComponent>({.Mode = CharacterControllerComponent::EMode::Physical});

    m_playerCam = GScene.CreateEntity("PlayerCamera", GScene.GetGameRoot());

    m_playerCam.set<CameraComponent>({.Priority = 1}).set<TransformComponent>({});

    GScene.CreateEntity("Enemy", GScene.GetGameRoot()).set<TransformComponent>({{-2.0f, 1.0f, 0.0f}})
            .set<VelocityComponent>({}).set<ColliderComponent>({
                .Shape = EPhysicsShape::Capsule, .Extents = {0.3f, 0.5f, 0.0}
            })
            .set<RigidbodyComponent>({
                .AllowedDOFs = EPhysicsDOF::TranslationX | EPhysicsDOF::TranslationY | EPhysicsDOF::TranslationZ
            })
            .set<GroundCheckComponent>({});;

    GEngine.AddSystem(std::bind_front(&Game::PlayerMovementSystem, this), ESystemPhase::FixedUpdate, 0,
                      ESystemGroup::Game);
    GEngine.AddSystem(std::bind_front(&Game::CameraUpdate, this), ESystemPhase::Update, 10,
                      ESystemGroup::Game);

    LogInfo("Layer number: {}", m_player.get<ColliderComponent>().Layer);
    LogInfo("Layer name: {}", GPhysics.GetLayerName(m_player.get<ColliderComponent>().Layer));
}

void Game::OnStop()
{
    GInput.SetRelativeMouseMode(false);
}

void Game::PlayerMovementSystem(flecs::world& world, float deltaTime)
{
    auto& transform = m_player.get_mut<TransformComponent>();

    float dx = GInput.GetMouseDeltaX() * m_mouseSensitivity;
    float dy = GInput.GetMouseDeltaY() * m_mouseSensitivity;

    if (dx != 0)
        transform.Rotate(TransformComponent::GlobalUp, dx, WorldSpace);

    if (dy != 0)
        m_currentPitch = std::clamp(m_currentPitch + dy, -m_pitchLimit, +m_pitchLimit);

    auto& cc = m_player.get_mut<CharacterControllerComponent>();
    if (!cc.Controller)
        return;

    Float3 dir{};
    if (GInput.IsDown(m_moveForward))
        dir += transform.GetForward();
    if (GInput.IsDown(m_moveBackward))
        dir -= transform.GetForward();
    if (GInput.IsDown(m_moveRight))
        dir += transform.GetRight();
    if (GInput.IsDown(m_moveLeft))
        dir -= transform.GetRight();

    if (Diligent::length(dir) > 0.000001f)
        dir = Diligent::normalize(dir);

    Float3 horizontal = dir * m_speed;
    Float3 vel = cc.Controller->GetLinearVelocity();
    cc.Controller->SetLinearVelocity({horizontal.x, vel.y, horizontal.z});

    if (GInput.IsDown(m_jump) && cc.Controller->IsGrounded())
        cc.Controller->Jump(m_jumpSpeed);
}

void Game::CameraUpdate(flecs::world& world, float deltaTime)
{
    auto& playerTransform = m_player.get_mut<TransformComponent>();
    auto& camTransform = m_playerCam.get_mut<TransformComponent>();

    camTransform.Position = playerTransform.Position + Float3(0.0f, m_eyeHeight, 0.0f);
    camTransform.Rotation = playerTransform.Rotation;
    camTransform.Rotate(TransformComponent::GlobalRight, m_currentPitch, LocalSpace);

    if (GInput.IsPressed(m_interact))
    {
        auto hit = GPhysics.Raycast(camTransform.Position, camTransform.GetForward(), m_interactionDistance, false,
                                    GPhysics.GetLayerMask("Default"), std::span(&m_player, 1));

        if (hit.Hit)
            LogInfo("[interact] hit {} at {:.2f}m", hit.Target.name().c_str(), hit.Distance);
        else
            LogInfo("[interact] nothing in range");
    }
}

extern "C"
{
GAME_API IGameModule* GetGameModule()
{
    return &GGame;
}
}
