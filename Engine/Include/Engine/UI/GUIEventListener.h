//
// Created by ivan on 11/5/26.
//

#pragma once
#include <RmlUi/Core/EventListener.h>
#include <functional>

namespace RTGDEngine
{
    class GUIEventListener final : public Rml::EventListener
    {
    public:
        explicit GUIEventListener(std::function<void(Rml::Event&)> callback);

        void ProcessEvent(Rml::Event& event) override;

    private:
        std::function<void(Rml::Event&)> m_callback = {};
    };
} // RTGDEngine
