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

namespace RTGDEngine
{
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

    private:
        Rml::Context* m_context = nullptr;
    };

    DECLARE_GLOBAL_SINGLETON(UISystem, GUI)
} // RTGDEngine
