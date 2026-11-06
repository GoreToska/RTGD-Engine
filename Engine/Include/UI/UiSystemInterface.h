//
// Created by ivan on 10/12/26.
//

#pragma once

#include <RmlUi/Core/SystemInterface.h>

#include "DefineGlobalFuncHelperMacros.h"
#include "DeviceContext.h"

namespace RTGDEngine
{
    class UiSystemInterface final : public Rml::SystemInterface
    {
    public:
        bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

        double GetElapsedTime() override;
    };
} // RTGDEngine
