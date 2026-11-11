//
// Created by ivan on 10/12/26.
//

#include "UI/UISystem.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include "UI/UiSystemInterface.h"
#include "UI/Views/UIView.h"
#include "Platform/InputEvent.h"
#include "UI/UiRenderInterface.h"
#include "Tools/Logger.h"
#include "Input/Key.h"
#include "RmlKeymap.h"
#include "UI/UIFileInterface.h"
#include "Event/EventBus.h"
#include "Event/Events.h"
#include "Input/InputSystem.h"

namespace RTGDEngine
{
    namespace
    {
        std::unique_ptr<UiSystemInterface> s_systemInterface;
        std::unique_ptr<UiRenderInterface> s_renderInterface;
        std::unique_ptr<UIFileInterface> s_fileInterface;
    }

    UISystem::~UISystem() = default;

    bool UISystem::Initialize(int width, int height)
    {
        s_systemInterface = std::make_unique<UiSystemInterface>();
        s_renderInterface = std::make_unique<UiRenderInterface>();
        s_fileInterface = std::make_unique<UIFileInterface>();

        if (!s_renderInterface->Initialize())
        {
            LogError("Failed to initialize render interface.");
            return false;
        }

        Rml::SetSystemInterface(s_systemInterface.get());
        Rml::SetRenderInterface(s_renderInterface.get());
        Rml::SetFileInterface(s_fileInterface.get());

        if (!Rml::Initialise())
        {
            LogError("Failed to initialize RmlUi.");
            return false;
        }

        m_context = Rml::CreateContext("main", {width, height});

        if (!Rml::LoadFontFace("Assets/Fonts/LatoLatin-Regular.ttf"))
            LogError("Failed to load font {}.", "Assets/Fonts/LatoLatin-Regular.ttf");

        if (!Rml::Debugger::Initialise(m_context))
            LogError("Failed to initialise RmlUi Debugger.");

        Rml::Debugger::SetVisible(false);

        m_inputModeSubscription = GEventBus().Subscribe(Events::OnInputModeChanged,
                                                        [this](const Events::InputModeChangedEvent& event)
                                                        {
                                                            if (event.Current == EInputMode::GameOnly && m_context)
                                                                m_context->ProcessMouseLeave();
                                                        });

        return true;
    }

    void UISystem::Shutdown()
    {
        if (m_inputModeSubscription != 0)
        {
            GEventBus().Unsubscribe(m_inputModeSubscription);
            m_inputModeSubscription = 0;
        }

        for (auto& view: m_views)
            view->Destroy();

        m_views.clear();
        m_pendingCloseViews.clear();

        Rml::Shutdown();
        m_context = nullptr;
        s_systemInterface.reset();
        s_renderInterface.reset();
        s_fileInterface.reset();
    }

    void UISystem::Update(float deltaTime)
    {
        for (size_t i = 0; i < m_views.size(); i++)
        {
            m_views[i]->OnUpdate(deltaTime);
        }

        DestroyPendingViews();

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

    bool UISystem::ProcessInput(const InputEvent& event)
    {
        if (!m_context)
            return false;

        int modifiers = GetKeyModifier(event);

        switch (event.Type)
        {
            case EInputEventType::MouseMove:
                return !m_context->ProcessMouseMove(event.X, event.Y, modifiers);

            case EInputEventType::MouseButton:
            {
                const int button = MouseButton(event.Button).ToRml();
                if (button < 0)
                    return false;
                return event.Down
                           ? !m_context->ProcessMouseButtonDown(button, modifiers)
                           : !m_context->ProcessMouseButtonUp(button, modifiers);
            }

            case EInputEventType::MouseWheel:
                return !m_context->ProcessMouseWheel(Rml::Vector2f(0.0f, -event.Wheel), modifiers);

            case EInputEventType::MouseLeave:
            {
                m_context->ProcessMouseLeave();
                return false;
            }

            case EInputEventType::Key:
            {
                const auto key = Key(event.Key).ToRml();
                if (key == Rml::Input::KI_UNKNOWN)
                    return false;
                return event.Down
                           ? !m_context->ProcessKeyDown(key, modifiers)
                           : !m_context->ProcessKeyUp(key, modifiers);
            }

            case EInputEventType::Text:
                return !m_context->ProcessTextInput(event.Text);
            case EInputEventType::FocusLost:
                return false;
        }

        return false;
    }

    void UISystem::Close(UIView* view)
    {
        if (!view)
            return;

        if (std::ranges::find(m_pendingCloseViews, view) != m_pendingCloseViews.end())
            return;

        m_pendingCloseViews.push_back(view);
    }

    bool UISystem::AddView(std::unique_ptr<UIView> view)
    {
        if (!m_context)
            return false;

        if (!view->Load(*m_context))
            return false;

        m_views.push_back(std::move(view));
        return true;
    }

    void UISystem::DestroyPendingViews()
    {
        auto pending = std::move(m_pendingCloseViews);
        m_pendingCloseViews.clear();

        for (auto& view: pending)
        {
            view->Destroy();
            std::erase_if(m_views, [view](std::unique_ptr<UIView>& v)
            {
                return view == v.get();
            });
        }

        pending.clear();
    }
} // RTGDEngine
