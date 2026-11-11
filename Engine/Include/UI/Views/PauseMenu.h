#pragma once
#include "UIView.h"

namespace RTGDEngine
{
    class PauseMenu : public UIView
    {
    public:
        PauseMenu();

    protected:
        void OnInitialize() override;
        void OnShow() override;
        void OnHide() override;
    };
} // RTGDEngine
