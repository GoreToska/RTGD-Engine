#pragma once

#include "Engine/EngineExport.h"

namespace RTGDEngine
{
	class IGameModule
	{
	public:
		virtual ~IGameModule() = default;
		virtual void Initialize() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;
		virtual void Shutdown() = 0;
	};

	typedef IGameModule* (*CreateGameModuleFunc)();
	typedef void (*DestroyGameModuleFunc)(IGameModule*);
}
