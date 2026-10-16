//
// Created by ivan on 10/12/26.
//

#pragma once
#include <memory>

#include "Tools/RTGDMacros.h"

namespace Rml
{
    class Context;
}

namespace Diligent
{
    struct IDeviceContext;
}

struct NativeWindowEvent;

namespace RTGDEngine
{
    struct InputEvent;
    class UiSystemInterface;
    class UiRenderInterfaceStub;

    class UISystem
    {
        DECLARE_SINGLETON(UISystem);

    public:
        ~UISystem();

        bool Initialize(int width, int height);

        void Shutdown();

        void Update();

        void Resize(int width, int height) const;

        void Render(Diligent::IDeviceContext& context);

        void ProcessInput(const InputEvent& event);

    private:
        Rml::Context* m_context = nullptr;
    };

    DECLARE_GLOBAL_SINGLETON(UISystem, GUI)
} // RTGDEngine
