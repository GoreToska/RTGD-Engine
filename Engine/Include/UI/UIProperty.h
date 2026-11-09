#pragma once
#include <functional>
#include <utility>

namespace RTGDEngine
{
    template <typename T>
    class UIProperty
    {
    public:
        UIProperty() = default;

        UIProperty(T value) : m_value(std::move(value))
        {
        }

        UIProperty(const UIProperty&) = delete;

        UIProperty& operator=(const UIProperty&) = delete;

        UIProperty& operator=(const T& value)
        {
            Set(value);
            return *this;
        }

        void Set(const T& value)
        {
            if (m_value == value)
                return;

            m_value = value;
            if (m_onChanged)
                m_onChanged();
        }

        const T& Get() const
        {
            return m_value;
        }

        operator const T&() const
        {
            return m_value;
        }

    private:
        friend class UIView;

        T m_value{};
        std::function<void()> m_onChanged;
    };
} // RTGDEngine
