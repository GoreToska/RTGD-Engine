//
// Created by ivan on 11/9/26.
//

#include "UI/Views/HUDView.h"

#include "Engine/Engine.h"

namespace RTGDEngine
{
    HUDView::HUDView() : UIView("Assets/UI/HUD.rml", "hud")
    {
    }

    void HUDView::OnBindModel()
    {
        UIView::OnBindModel();
        BindData("fps", m_fps);
        BindData("frameMs", m_frameMs);
        BindData("status", m_status);
        BindData("showDetails", m_showDetails);
    }

    void HUDView::OnInitialize()
    {
        UIView::OnInitialize();

        On("toggle-details", "click", [this](Rml::Event&)
        {
            m_showDetails = !m_showDetails;
        });
    }

    void HUDView::OnUpdate(float deltaTime)
    {
        UIView::OnUpdate(deltaTime);

        m_fps = static_cast<int>(1.0f / GEngine().GetDeltaTime());
        m_frameMs = GEngine().GetDeltaTime() * 1000.0f;
    }
} // RTGDEngine
