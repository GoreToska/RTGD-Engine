//
// Created by ivan on 11/5/26.
//

#include "../../Include/UI/GUIEventListener.h"

namespace RTGDEngine
{
    GUIEventListener::GUIEventListener(std::function<void(Rml::Event&)> callback)
        : m_callback(std::move(callback))
    {
    }

    void GUIEventListener::ProcessEvent(Rml::Event& event)
    {
        if (m_callback)
            m_callback(event);
    }
} // RTGDEngine
