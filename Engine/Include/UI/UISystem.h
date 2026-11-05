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

namespace RTGDEngine
{
    class UIView;
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

        bool ProcessInput(const InputEvent& event);

        template<typename T, typename... Args>
        T* Open(Args&&... args)
        {
        }

        void Close(UIView* view);

    private:
        Rml::Context* m_context = nullptr;
    };

    DECLARE_GLOBAL_SINGLETON(UISystem, GUI)
} // RTGDEngine
