// Game/src/Game.h
#pragma once

#include "Engine/IGameModule.h"
#include "Tools/Alias.h"

class Game : public RTGDEngine::IGameModule {
public:
    static Game &Instance();

    void Initialize() override;

    void Update(float deltaTime) override;

    void Shutdown() override;

private:
    bool m_isInitialized = false;

    ActionID m_moveForward;
    ActionID m_moveBackward;
    ActionID m_moveLeft;
    ActionID m_moveRight;

    Entity m_player;
    float m_speed = 1.0f;
};
