#include "UI/Views/PauseMenu.h"

#include "Event/EventBus.h"
#include "Event/Events.h"
#include "Input/InputSystem.h"

namespace RTGDEngine
{
    PauseMenu::PauseMenu() : UIView(UIViewConfig("Assets/UI/PauseMenu.rml", "", UILayers::Menu))
    {
    }

    void PauseMenu::OnInitialize()
    {
        On("resume-button", "click", [this](Rml::Event&)
        {
            Close();
        });

        On("quit-button", "click", [](Rml::Event&)
        {
            GEventBus().Emit(Events::OnQuitRequested, {});
        });

        OnKey(gainput::KeyEscape, [this]
        {
            Close();
        });
    }

    void PauseMenu::OnShow()
    {
        GInput().SetInputMode(EInputMode::UIOnly);
    }

    void PauseMenu::OnHide()
    {
        GInput().SetInputMode(EInputMode::GameAndUI);
    }
} // RTGDEngine
