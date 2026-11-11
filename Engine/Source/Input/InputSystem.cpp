//
// Created by gorev on 13.03.2026.
//

#include "Input/InputSystem.h"

#include "Event/EventBus.h"
#include "Event/Events.h"
#include "Input/KeyboardDevice.h"
#include "Input/MouseDevice.h"
#include "Platform/IPlatformWindow.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    void InputSystem::CreateInputMapping()
    {
        m_map = std::make_unique<InputMap>(m_manager);

        using A = EInputAction;
        m_map->MapBool(ID(A::MoveForward), m_keyboard, KeyW);
        m_map->MapBool(ID(A::MoveBackward), m_keyboard, KeyS);
        m_map->MapBool(ID(A::MoveLeft), m_keyboard, KeyA);
        m_map->MapBool(ID(A::MoveRight), m_keyboard, KeyD);
        m_map->MapBool(ID(A::MoveUp), m_keyboard, KeyE);
        m_map->MapBool(ID(A::MoveDown), m_keyboard, KeyQ);
        m_map->MapBool(ID(A::SpeedBoost), m_keyboard, KeyShiftL);
        m_map->MapBool(ID(A::Escape), m_keyboard, KeyEscape);

        m_map->MapBool(ID(A::CtrlLeft), m_keyboard, KeyCtrlL);
        m_map->MapBool(ID(A::ReloadGameModule), m_keyboard, KeyR);
        m_map->MapBool(ID(A::TogglePlayMode), m_keyboard, KeyP);
        m_map->MapBool(ID(A::SaveScene), m_keyboard, KeyS);

        m_map->MapBool(ID(A::MouseRight), m_mouse, MouseButtonRight);
    }

    // TODO: need to initialize once and just switch focus of windows
    void InputSystem::InitializeInputForWindow(IPlatformWindow* handle)
    {
        m_manager.SetDisplaySize(handle->GetHandle().width, handle->GetHandle().height);

        CreateKeyboardDevice();
        CreateMouseDevice();

        CreateInputMapping();
        SubscribeToInputModeChanges();

        LogInfo("InputSystem initialized ({}x{})", handle->GetHandle().width, handle->GetHandle().height);
    }

    void InputSystem::AddWindowHandle(IPlatformWindow* window)
    {
        // TODO: add to vector of current windows

        m_platformWindow = window;

        InitializeInputForWindow(window);
    }

    void InputSystem::HandleInputEvent(const InputEvent& event)
    {
        switch (event.Type)
        {
            case EInputEventType::MouseButton:
                InjectMouseButton(event.Button, event.Down);
                break;
            case EInputEventType::Key:
                InjectKey(event.Key, event.Down);
                break;
            case EInputEventType::FocusLost:
                ReleaseAllButtons();
                break;

            default:
                break;
        }
    }

    void InputSystem::Update()
    {
        m_manager.Update();

        if (m_inputMode != EInputMode::UIOnly && IsPressed(EInputAction::MouseRight))
            SetRelativeMouseMode(true);
        if (IsReleased(EInputAction::MouseRight) || IsPressed(EInputAction::Escape))
            SetRelativeMouseMode(false);

        if (!m_platformWindow || !m_mouseCaptured)
            return;

        float dx = 0.0f, dy = 0.0f;
        if (m_platformWindow->GetMouseDelta(dx, dy))
        {
            m_mouseDeltaX = dx;
            m_mouseDeltaY = dy;
        }
    }

    void InputSystem::PostUpdate()
    {
        m_mouseDeltaX = 0;
        m_mouseDeltaY = 0;
    }

    void InputSystem::Resize(const int width, const int height)
    {
        m_manager.SetDisplaySize(width, height);
    }

    bool InputSystem::IsDown(const EInputAction action) const
    {
        return m_map->GetBool(ID(action));
    }

    bool InputSystem::IsDown(ActionID action) const
    {
        return m_map->GetBool(action);
    }

    bool InputSystem::IsPressed(ActionID action) const
    {
        return m_map->GetBoolIsNew(action);
    }

    bool InputSystem::IsReleased(ActionID action) const
    {
        return m_map->GetBoolWasDown(action);
    }

    float InputSystem::GetAxis(ActionID action) const
    {
        return m_map->GetFloat(action);
    }

    ActionID InputSystem::RegisterAction(const std::string& name)
    {
        if (auto it = m_customActions.find(name); it != m_customActions.end())
            return it->second;

        ActionID id = m_nextActionID++;
        m_customActions.emplace(name, id);
        return id;
    }

    void InputSystem::BindKey(ActionID action, Key key) const
    {
        m_map->MapBool(action, m_keyboard, key);
    }

    void InputSystem::BindMouseButton(ActionID action, MouseButton button) const
    {
        m_map->MapBool(action, m_mouse, button);
    }

    void InputSystem::SetInputMode(EInputMode mode)
    {
        if (m_inputMode == mode)
            return;

        auto previousMode = m_inputMode;
        m_inputMode = mode;

        GEventBus().Emit(Events::OnInputModeChanged, {previousMode, mode}, {});
    }

    void InputSystem::SubscribeToInputModeChanges()
    {
        if (m_inputModeSubscription != 0)
            return;

        m_inputModeSubscription = GEventBus().Subscribe(Events::OnInputModeChanged,
                                                        [this](const Events::InputModeChangedEvent& event)
                                                        {
                                                            if (event.Current != EInputMode::UIOnly)
                                                                return;

                                                            SetRelativeMouseMode(false);
                                                            ReleaseAllButtons();
                                                        });
    }

    void InputSystem::ReleaseAllButtons() const
    {
        for (int i = 0; i < KeyCount_; i++)
            InjectKey(static_cast<Key>(i), false);

        for (int i = 0; i < MouseButtonCount_; i++)
            InjectMouseButton(static_cast<MouseButton>(i), false);
    }

    EInputMode InputSystem::GetInputMode() const
    {
        return m_inputMode;
    }

    bool InputSystem::IsPressed(const EInputAction action) const
    {
        return m_map->GetBoolIsNew(ID(action));
    }

    bool InputSystem::IsReleased(const EInputAction action) const
    {
        return m_map->GetBoolWasDown(ID(action));
    }

    float InputSystem::GetAxis(EInputAction action) const
    {
        return m_map->GetFloat(ID(action));
    }

    bool InputSystem::IsMouseCaptured() const
    {
        return m_mouseCaptured;
    }

    float InputSystem::GetMouseDeltaX() const
    {
        return m_mouseDeltaX;
    }

    float InputSystem::GetMouseDeltaY() const
    {
        return m_mouseDeltaY;
    }

    void InputSystem::InjectKey(Key key, bool down) const
    {
        if (m_injectKeyboard)
            m_injectKeyboard->InjectButton(key, down);
    }

    void InputSystem::InjectMouseButton(MouseButton button, bool down) const
    {
        if (m_injectMouseButton)
            m_injectMouseButton->InjectButton(button, down);
    }

    void InputSystem::SetRelativeMouseMode(bool relative)
    {
        if (!m_platformWindow || m_mouseCaptured == relative)
            return;

        m_mouseCaptured = relative;
        m_platformWindow->SetRelativeMouseMode(relative);
    }

    void InputSystem::InjectMouseMove(float dx, float dy) const
    {
        if (m_platformWindow)
            m_platformWindow->InjectMouseMove(dx, dy);
    }

    void InputSystem::WarpCursorToCenter() const
    {
        if (m_platformWindow)
            m_platformWindow->WarpCursorToCenter();
    }

    void InputSystem::SetCursorVisible(bool visible) const
    {
        if (m_platformWindow)
            m_platformWindow->SetCursorVisible(visible);
    }

    void InputSystem::CreateKeyboardDevice()
    {
        m_keyboard = m_manager.CreateDevice<KeyboardDevice>();
        m_injectKeyboard = dynamic_cast<IInjectableButton*>(m_manager.GetDevice(m_keyboard));
    }

    void InputSystem::CreateMouseDevice()
    {
        m_mouse = m_manager.CreateDevice<MouseDevice>();
        m_injectMouseButton = dynamic_cast<IInjectableButton*>(m_manager.GetDevice(m_mouse));
    }
}
