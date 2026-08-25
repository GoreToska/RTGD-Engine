// Game/src/Game.h
#pragma once

#include "Engine/IGameModule.h"
#include "Tools/Alias.h"
#include "Tools/RTGDMacros.h"

class Game : public IGameModule {
public:
    static Game &Instance();

    void Initialize() override;

    void Update(float deltaTime) override;

    void Shutdown() override;

    void OnStart() override;

    void OnStop() override;

private:
    void PlayerMovementSystem(flecs::world &world, float deltaTime);

    void CameraUpdate(flecs::world &world, float deltaTime);

    bool m_isInitialized = false;

    ActionID m_moveForward;
    ActionID m_moveBackward;
    ActionID m_moveLeft;
    ActionID m_moveRight;

    Entity m_player;
    Entity m_playerCam;

    Float3 m_cameraOffset = {0.0f, 2.0f, -2.0f};
    float m_speed = 0.1f;
};

DECLARE_GLOBAL_SINGLETON(Game, GGame)
