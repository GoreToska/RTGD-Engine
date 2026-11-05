//
// Created by ivan on 10/12/26.
//

#include "UI/UISystem.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include "AssetLoader/PathResolve.h"
#include "Engine/UI/UiSystemInterface.h"
#include "Platform/InputEvent.h"
#include "UI/UiRenderInterface.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    namespace
    {
        std::unique_ptr<UiSystemInterface> s_systemInterface;
        std::unique_ptr<UiRenderInterface> s_renderInterface;

        int GetKeyModifier(const InputEvent& event)
        {
            int modifier = 0;
            if (event.Alt)
                modifier |= Rml::Input::KM_ALT;
            if (event.Shift)
                modifier |= Rml::Input::KM_SHIFT;
            if (event.Ctrl)
                modifier |= Rml::Input::KM_CTRL;
            return modifier;
        }

        Rml::Input::KeyIdentifier ToRmlKey(const gainput::Key key)
        {
            using namespace Rml::Input;

            static_assert(gainput::KeyZ - gainput::KeyA == KI_Z - KI_A);
            static_assert(gainput::Key9 - gainput::Key0 == KI_9 - KI_0);
            static_assert(gainput::KeyF19 - gainput::KeyF1 == KI_F19 - KI_F1);

            if (key >= gainput::KeyA && key <= gainput::KeyZ)
                return static_cast<KeyIdentifier>(KI_A + (key - gainput::KeyA));
            if (key >= gainput::Key0 && key <= gainput::Key9)
                return static_cast<KeyIdentifier>(KI_0 + (key - gainput::Key0));
            if (key >= gainput::KeyF1 && key <= gainput::KeyF19)
                return static_cast<KeyIdentifier>(KI_F1 + (key - gainput::KeyF1));

            switch (key)
            {
                // Editing / control
                case gainput::KeyEscape:
                    return KI_ESCAPE;
                case gainput::KeyBackSpace:
                    return KI_BACK;
                case gainput::KeyTab:
                    return KI_TAB;
                case gainput::KeyReturn:
                    return KI_RETURN;
                case gainput::KeySpace:
                    return KI_SPACE;
                case gainput::KeyInsert:
                    return KI_INSERT;
                case gainput::KeyDelete:
                    return KI_DELETE;
                case gainput::KeyClear:
                    return KI_CLEAR;

                // Navigation
                case gainput::KeyLeft:
                    return KI_LEFT;
                case gainput::KeyRight:
                    return KI_RIGHT;
                case gainput::KeyUp:
                    return KI_UP;
                case gainput::KeyDown:
                    return KI_DOWN;
                case gainput::KeyHome:
                    return KI_HOME;
                case gainput::KeyEnd:
                    return KI_END;
                case gainput::KeyPageUp:
                    return KI_PRIOR;
                case gainput::KeyPageDown:
                    return KI_NEXT;

                // Punctuation (US layout positions)
                case gainput::KeySemicolon:
                    return KI_OEM_1;
                case gainput::KeyEqual:
                    return KI_OEM_PLUS;
                case gainput::KeyComma:
                    return KI_OEM_COMMA;
                case gainput::KeyMinus:
                    return KI_OEM_MINUS;
                case gainput::KeyPeriod:
                    return KI_OEM_PERIOD;
                case gainput::KeySlash:
                    return KI_OEM_2;
                case gainput::KeyGrave:
                    return KI_OEM_3;
                case gainput::KeyBracketLeft:
                    return KI_OEM_4;
                case gainput::KeyBackslash:
                    return KI_OEM_5;
                case gainput::KeyBracketRight:
                    return KI_OEM_6;
                case gainput::KeyApostrophe:
                    return KI_OEM_7;
                case gainput::KeyLess:
                    return KI_OEM_102;

                // Numpad (gainput names keys by their numlock-off function)
                case gainput::KeyKpInsert:
                    return KI_NUMPAD0;
                case gainput::KeyKpEnd:
                    return KI_NUMPAD1;
                case gainput::KeyKpDown:
                    return KI_NUMPAD2;
                case gainput::KeyKpPageDown:
                    return KI_NUMPAD3;
                case gainput::KeyKpLeft:
                    return KI_NUMPAD4;
                case gainput::KeyKpBegin:
                    return KI_NUMPAD5;
                case gainput::KeyKpRight:
                    return KI_NUMPAD6;
                case gainput::KeyKpHome:
                    return KI_NUMPAD7;
                case gainput::KeyKpUp:
                    return KI_NUMPAD8;
                case gainput::KeyKpPageUp:
                    return KI_NUMPAD9;
                case gainput::KeyKpDelete:
                    return KI_DECIMAL;
                case gainput::KeyKpEnter:
                    return KI_NUMPADENTER;
                case gainput::KeyKpAdd:
                    return KI_ADD;
                case gainput::KeyKpSubtract:
                    return KI_SUBTRACT;
                case gainput::KeyKpMultiply:
                    return KI_MULTIPLY;
                case gainput::KeyKpDivide:
                    return KI_DIVIDE;
                case gainput::KeyKpEqual:
                    return KI_OEM_NEC_EQUAL;

                // Modifiers / locks
                case gainput::KeyShiftL:
                    return KI_LSHIFT;
                case gainput::KeyShiftR:
                    return KI_RSHIFT;
                case gainput::KeyCtrlL:
                    return KI_LCONTROL;
                case gainput::KeyCtrlR:
                    return KI_RCONTROL;
                case gainput::KeyAltL:
                    return KI_LMENU;
                case gainput::KeyAltR:
                    return KI_RMENU;
                case gainput::KeyAltGr:
                    return KI_RMENU;
                case gainput::KeySuperL:
                    return KI_LWIN;
                case gainput::KeySuperR:
                    return KI_RWIN;
                case gainput::KeyMenu:
                    return KI_APPS;
                case gainput::KeyCapsLock:
                    return KI_CAPITAL;
                case gainput::KeyNumLock:
                    return KI_NUMLOCK;
                case gainput::KeyScrollLock:
                    return KI_SCROLL;

                // System
                case gainput::KeyPrint:
                    return KI_SNAPSHOT;
                case gainput::KeySysRq:
                    return KI_SNAPSHOT;
                case gainput::KeyBreak:
                    return KI_PAUSE;

                // Media
                case gainput::KeyMute:
                    return KI_VOLUME_MUTE;
                case gainput::KeyVolumeDown:
                    return KI_VOLUME_DOWN;
                case gainput::KeyVolumeUp:
                    return KI_VOLUME_UP;
                case gainput::KeyMediaPlayPause:
                    return KI_MEDIA_PLAY_PAUSE;
                case gainput::KeyMediaStop:
                    return KI_MEDIA_STOP;
                case gainput::KeyMediaNext:
                    return KI_MEDIA_NEXT_TRACK;
                case gainput::KeyMediaPrevious:
                    return KI_MEDIA_PREV_TRACK;

                default:
                    return KI_UNKNOWN;
            }
        }

        int ToRmlMouseButton(const gainput::MouseButton button)
        {
            switch (button)
            {
                case gainput::MouseButtonLeft:
                    return 0;
                case gainput::MouseButtonRight:
                    return 1;
                case gainput::MouseButtonMiddle:
                    return 2;
                default:
                    return -1;
            }
        }
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
                const int button = ToRmlMouseButton(event.Button);
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
                auto key = ToRmlKey(event.Key);
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
} // RTGDEngine
