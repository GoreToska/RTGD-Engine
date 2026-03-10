// Game/src/Game.h
#pragma once

#include "Engine/IGameModule.h"

class Game : public Engine::IGameModule
{
public:
    static Game& Instance();

    void Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    //Engine::IEngineInterface* GetEngine() const { return m_engine; }

private:
    //Engine::IEngineInterface* m_engine = nullptr;

    bool m_isInitialized = false;
};