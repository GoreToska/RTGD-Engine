//
// Created by ivan on 11/6/26.
//

#pragma once
#include "Engine/UI/UIView.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    class TestView : public UIView
    {
    public:
        TestView() : UIView("Assets/UI/Test.rml")
        {
        }

        void OnInitialize() override
        {
            On("hello-button", "click", [this](Rml::Event&) { LogInfo("Hello!"); });
            On("close-button", "click", [this](Rml::Event&)
            {
                LogInfo("Close!");
                this->Close();
            });
        }
    };
}
