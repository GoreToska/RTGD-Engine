//
// Created by ivan on 10/12/26.
//

#pragma once

#include <RmlUi/Core/SystemInterface.h>

namespace RTGDEngine
{
    class UiSystemInterface : public Rml::SystemInterface
    {
    public:
        bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

        double GetElapsedTime() override;
    };
} // RTGDEngine
