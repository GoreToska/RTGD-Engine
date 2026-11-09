//
// Created by ivan on 11/9/26.
//

#pragma once
#include "UIView.h"

namespace RTGDEngine
{
    class HUDView : public UIView
    {
    public:
        HUDView();

    protected:
        void OnBindModel() override;
        void OnInitialize() override;
        void OnUpdate(float deltaTime) override;

    private:
        UIProperty<int> m_fps;
        UIProperty<float> m_frameMs;
        UIProperty<bool> m_showDetails = false;
        UIProperty<std::string> m_status = std::string("Debug HUD");

        float m_accumulatedTime = 0.0f;
        int m_accumulatedFrames = 0;
    };
} // RTGDEngine
