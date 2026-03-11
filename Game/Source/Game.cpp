#include "pch.h"
#include "Game.h"
#include <iostream>

Game& Game::Instance()
{
	static Game instance;
	return instance;
}

void Game::Initialize()
{
	std::cout << "[Game] Initialized with Engine interface!" << std::endl;
}

void Game::Update(float deltaTime)
{
}

void Game::Render()
{
}

void Game::Shutdown()
{
}

extern "C"
{
	__declspec(dllexport) RTGDEngine::IGameModule* CreateGameModule()
	{
		return &Game::Instance();
	}

	__declspec(dllexport) void DestroyGameModule(RTGDEngine::IGameModule* module)
	{
		module->Shutdown();
	}
}