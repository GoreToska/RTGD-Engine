//
// Created by ivan on 11/5/26.
//

#include "../../Include/UI/Views/UIView.h"

#include <RmlUi/Core.h>

#include <utility>

#include "AssetLoader/PathResolve.h"
#include "../../Include/UI/GUIEventListener.h"
#include "Tools/Logger.h"
#include "UI/UISystem.h"

namespace RTGDEngine
{
    UIView::UIView(const std::string& relativeDocumentPath)
        : m_documentPath(relativeDocumentPath)
    {
    }

    UIView::~UIView() = default;

    Rml::Element* UIView::Bind(const std::string& id)
    {
        Rml::Element* element = m_document ? m_document->GetElementById(id) : nullptr;
        if (!element)
        {
            LogError("Failed to bind element '{}' to '{}'.", id, m_documentPath);
        }
        return element;
    }

    void UIView::On(Rml::Element* element, const std::string& event, std::function<void(Rml::Event&)> callback)
    {
        if (!element)
            return;

        auto listener = std::make_unique<GUIEventListener>(std::move(callback));
        element->AddEventListener(event, listener.get());
        m_listeners.push_back({element, event, std::move(listener)});
    }

    void UIView::On(const std::string& id, const std::string& event, std::function<void(Rml::Event&)> callback)
    {
        On(Bind(id), event, std::move(callback));
    }

    void UIView::Close()
    {
        GUI().Close(this);
    }

    Rml::ElementDocument* UIView::GetDocument() const
    {
        return m_document;
    }

    bool UIView::Load(Rml::Context& context)
    {
        m_document = context.LoadDocument(m_documentPath);

        if (!m_document)
        {
            LogError("Failed to load view document '{}'.", m_documentPath);
            return false;
        }

        OnInitialize();

        m_document->Show();
        OnShow();
        return true;
    }

    void UIView::Destroy()
    {
        for (auto& element: m_listeners)
        {
            element.Element->RemoveEventListener(element.Event, element.Listener.get());
        }

        OnDestroy();

        if (m_document)
        {
            m_document->Hide();
            OnHide();
            m_document->Close();
            m_document = nullptr;
        }

        m_listeners.clear();
    }
} // RTGDEditor
