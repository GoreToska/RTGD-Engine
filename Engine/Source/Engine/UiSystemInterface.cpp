//
// Created by ivan on 10/12/26.
//

#include "../../Include/Engine/UI/UiSystemInterface.h"

#include "Engine/Engine.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    bool UiSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
    {
        switch (type)
        {
            case Rml::Log::LT_ERROR:
            case Rml::Log::LT_ASSERT:
                LogError("[RmlUi] {}", message);
                break;
            case Rml::Log::LT_WARNING:
                LogWarn("[RmlUi] {}", message);
                break;
            case Rml::Log::LT_DEBUG:
                LogTrace("[RmlUi] {}", message);
                break;
            default:
                LogInfo("[RmlUi] {}", message);
        }

        return true;
    }

    double UiSystemInterface::GetElapsedTime()
    {
        return GEngine().GetTime();
    }
} // RTGDEngine
