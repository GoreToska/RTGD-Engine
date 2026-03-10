#pragma once

namespace Engine
{
	class PhysicsSystem {};
	class RenderSystem {};
	class InputSystem {};

	class IEngineInterface
	{
	public:
		virtual ~IEngineInterface() = default;

		/*virtual PhysicsSystem* GetPhysicsSystem() = 0;
		virtual RenderSystem* GetRenderSystem() = 0;
		virtual InputSystem* GetInputSystem() = 0;*/
	};
}


