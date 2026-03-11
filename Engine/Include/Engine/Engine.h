#pragma once

#include <Windows.h>
#include <memory>
#include <string>

#include "Engine/IGameModule.h"
#include "Engine/IEngineInterface.h"


#ifdef _WIN32
#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)  
#else
#define ENGINE_API __declspec(dllimport) 
#endif
#else
#define ENGINE_API
#endif

#ifdef __cplusplus
extern "C" {

#endif

	ENGINE_API bool Engine_Initialize(void* hwnd);
	ENGINE_API void Engine_Run();
	ENGINE_API void Engine_Shutdown();
	ENGINE_API bool Engine_LoadGameModule(const char* dllPath);
	ENGINE_API void Engine_Update(float deltaTime);
	ENGINE_API void Engine_Render();

#ifdef __cplusplus
}
#endif

namespace RTGDEngine
{
	class Engine : public IEngineInterface
	{
	public:
		static Engine& Instance();

		bool Initialize(void* hwnd);
		void Run();
		void Shutdown();
		bool LoadGameModule(const std::string& dllPath);
		void Update(float deltaTime);
		void Render();

	private:
		void* m_hwnd = nullptr;
		std::unique_ptr<IGameModule> m_gameModule;

		HMODULE m_dllHandle = nullptr;
		CreateGameModuleFunc m_createFunc = nullptr;
		DestroyGameModuleFunc m_destroyFunc = nullptr;
	};
}