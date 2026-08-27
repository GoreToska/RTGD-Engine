// Game/src/Game.h
#pragma once

#include "Engine/IGameModule.h"
#include "Tools/Alias.h"
#include "Tools/RTGDMacros.h"

class Game : public IGameModule
{
public:
    static Game& Instance();

    void Initialize() override;

    void Update(float deltaTime) override;

    void Shutdown() override;


    void OnStart() override;

    void OnStop() override;

private:
    void SetupInput();

    void PlayerMovementSystem(flecs::world& world, float deltaTime);

    void CameraUpdate(flecs::world& world, float deltaTime);

    bool m_isInitialized = false;

    ActionID m_moveForward;
    ActionID m_moveBackward;
    ActionID m_moveLeft;
    ActionID m_moveRight;
    ActionID m_interact;
    ActionID m_jump;

    Entity m_player;
    Entity m_playerCam;

    float m_eyeHeight = 0.8f;
    float m_currentPitch = 0.0f;
    float m_pitchLimit = 89.0f;
    float m_mouseSensitivity = 0.15f;
    float m_speed = 2.f;
    float m_interactionDistance = 100.f;
    float m_jumpSpeed = 5.f;
};

DECLARE_GLOBAL_SINGLETON(Game, GGame)
