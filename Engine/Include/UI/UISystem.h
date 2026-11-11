//
// Created by ivan on 10/12/26.
//

#pragma once
#include <memory>
#include <vector>

#include "Tools/RTGDMacros.h"
#include "Views/UIView.h"

namespace Rml
{
    class Context;
}

namespace Diligent
{
    struct IDeviceContext;
}

namespace RTGDEngine
{
    class UIFileInterface;
    class UIView;
    struct InputEvent;
    class UiSystemInterface;
    class UiRenderInterfaceStub;

    class UISystem
    {
        DECLARE_SINGLETON(UISystem);

    public:
        ~UISystem();

        bool Initialize(int width, int height);

        void Shutdown();

        void Update(float deltaTime);

        void Resize(int width, int height) const;

        void Render(Diligent::IDeviceContext& context);

        bool ProcessInput(const InputEvent& event);

        template<typename T, typename... Args>
        T* Open(Args&&... args)
        {
            static_assert(std::is_base_of_v<UIView, T>, "T must be derived from UIView");
            auto view = std::make_unique<T>(std::forward<Args>(args)...);
            T* raw = view.get();
            return AddView(std::move(view)) ? raw : nullptr;
        }

        void Close(UIView* view);

    private:
        bool AddView(std::unique_ptr<UIView> view);

        void DestroyPendingViews();

        Rml::Context* m_context = nullptr;
        uint64_t m_inputModeSubscription = 0;
        std::vector<std::unique_ptr<UIView>> m_views;
        std::vector<UIView*> m_pendingCloseViews = {};
    };

    DECLARE_GLOBAL_SINGLETON(UISystem, GUI)
} // RTGDEngine
