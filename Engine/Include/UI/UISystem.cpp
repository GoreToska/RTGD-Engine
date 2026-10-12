//
// Created by ivan on 10/12/26.
//

#include "UISystem.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include "AssetLoader/PathResolve.h"
#include "Engine/UI/UiRenderInterfaceStub.h"
#include "Engine/UI/UiSystemInterface.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    namespace
    {
        std::unique_ptr<UiSystemInterface> s_systemInterface;
        std::unique_ptr<UiRenderInterfaceStub> s_renderInterface;
    }

    UISystem::~UISystem() = default;

    bool UISystem::Initialize(int width, int height)
    {
        s_systemInterface = std::make_unique<UiSystemInterface>();
        s_renderInterface = std::make_unique<UiRenderInterfaceStub>();

        Rml::SetSystemInterface(s_systemInterface.get());
        Rml::SetRenderInterface(s_renderInterface.get());

        if (!Rml::Initialise())
        {
            LogError("Failed to initialize RmlUi.");
            return false;
        }

        m_context = Rml::CreateContext("main", {width, height});

        if (!Rml::LoadFontFace(GetAbsolutePath("Assets/Fonts/LatoLatin-Regular.ttf")))
            LogError("Failed to load font {}.", "Assets/Fonts/LatoLatin-Regular.ttf");

        if (!Rml::Debugger::Initialise(m_context))
            LogError("Failed to initialise RmlUi Debugger.");

        if (auto* doc = m_context->LoadDocumentFromMemory(
            "<rml><head><style>body{font-family:LatoLatin;}</style></head><body>Hello</body></rml>"))
            doc->Show();

        return true;
    }

    void UISystem::Shutdown()
    {
        Rml::Shutdown();
        m_context = nullptr;
        s_systemInterface.reset();
        s_renderInterface.reset();
    }

    void UISystem::Update()
    {
        if (m_context)
            m_context->Update();
    }

    void UISystem::Resize(int width, int height) const
    {
        m_context->SetDimensions({width, height});
    }
} // RTGDEngine
