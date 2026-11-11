#include "Input/Key.h"

#include <array>

#include <RmlUi/Core/Input.h>

namespace RTGDEngine
{
    namespace
    {
        const std::array<gainput::Key, 256>& RmlToGainputTable()
        {
            static const auto table = []
            {
                std::array<gainput::Key, 256> result;
                result.fill(gainput::KeyCount_);
                for (int i = 0; i < gainput::KeyCount_; ++i)
                {
                    const auto key = static_cast<gainput::Key>(i);
                    const auto rml = Key(key).ToRml();
                    if (rml != Rml::Input::KI_UNKNOWN && result[rml] == gainput::KeyCount_)
                        result[rml] = key;
                }
                return result;
            }();
            return table;
        }
    }

    Key::Key(Rml::Input::KeyIdentifier key)
        : m_key(RmlToGainputTable()[key])
    {
    }

    Rml::Input::KeyIdentifier Key::ToRml() const
    {
        const gainput::Key key = m_key;
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

    MouseButton MouseButton::FromRml(int rmlButtonIndex)
    {
        switch (rmlButtonIndex)
        {
            case 0:
                return gainput::MouseButtonLeft;
            case 1:
                return gainput::MouseButtonRight;
            case 2:
                return gainput::MouseButtonMiddle;
            default:
                return {};
        }
    }

    int MouseButton::ToRml() const
    {
        switch (m_button)
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
} // RTGDEngine
