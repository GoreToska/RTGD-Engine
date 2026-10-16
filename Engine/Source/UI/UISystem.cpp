//
// Created by ivan on 10/12/26.
//

#include "UI/UISystem.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include "AssetLoader/PathResolve.h"
#include "Engine/UI/UiSystemInterface.h"
#include "UI/UiRenderInterface.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    namespace
    {
        std::unique_ptr<UiSystemInterface> s_systemInterface;
        std::unique_ptr<UiRenderInterface> s_renderInterface;
    }

    UISystem::~UISystem() = default;

    bool UISystem::Initialize(int width, int height)
    {
        s_systemInterface = std::make_unique<UiSystemInterface>();
        s_renderInterface = std::make_unique<UiRenderInterface>();

        if (!s_renderInterface->Initialize())
        {
            LogError("Failed to initialize render interface.");
            return false;
        }

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

        Rml::Debugger::SetVisible(true);

        if (auto* doc = m_context->LoadDocumentFromMemory(R"(
            <rml><head><style>
            body, div { display: block; }
            body { font-family: LatoLatin; font-size: 48dp; color: white; }
            div { width: 200dp; height: 100dp; }
            </style></head>
            <body>
            Hello
            <div style="background-color: #ff000080;"></div>
            <div style="background-color: #808080;"></div>
            </body></rml>)"))
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

    void UISystem::Render(Diligent::IDeviceContext& context)
    {
        if (!m_context)
            return;

        const auto dimensions = m_context->GetDimensions();
        s_renderInterface->BeginFrame(context, dimensions.x, dimensions.y);
        m_context->Render();
    }

    void UISystem::ProcessInput(const InputEvent& event)
    {

    }
} // RTGDEngine
