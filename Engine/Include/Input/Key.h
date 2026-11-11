#pragma once
#include <gainput/gainput.h>

#include "Engine/EngineExport.h"

namespace Rml::Input
{
    enum KeyIdentifier : unsigned char;
}

namespace RTGDEngine
{
    class ENGINE_API Key
    {
    public:
        constexpr Key() = default;

        constexpr Key(gainput::Key key) : m_key(key)
        {
        }

        explicit Key(Rml::Input::KeyIdentifier key);

        constexpr gainput::Key ToGainput() const
        {
            return m_key;
        }

        Rml::Input::KeyIdentifier ToRml() const;

        constexpr bool IsValid() const
        {
            return m_key != gainput::KeyCount_;
        }

        constexpr bool operator==(const Key&) const = default;

    private:
        gainput::Key m_key = gainput::KeyCount_;
    };

    class ENGINE_API MouseButton
    {
    public:
        constexpr MouseButton() = default;

        constexpr MouseButton(gainput::MouseButton button) : m_button(button)
        {
        }

        static MouseButton FromRml(int rmlButtonIndex);

        constexpr gainput::MouseButton ToGainput() const
        {
            return m_button;
        }

        int ToRml() const;

        constexpr bool IsValid() const
        {
            return m_button != gainput::MouseButtonCount_;
        }

        constexpr bool operator==(const MouseButton&) const = default;

    private:
        gainput::MouseButton m_button = gainput::MouseButtonCount_;
    };
} // RTGDEngine
