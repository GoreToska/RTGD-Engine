//
// Created by ivan on 11/5/26.
//

#include "UI/Views/UIView.h"

#include <RmlUi/Core.h>

#include <utility>

#include "AssetLoader/PathResolve.h"
#include "UI/GUIEventListener.h"
#include "Tools/Logger.h"
#include "UI/UISystem.h"

namespace RTGDEngine
{
    UIView::UIView(UIViewConfig config)
        : m_config(std::move(config))
    {
    }

    UIView::~UIView() = default;

    UILayer UIView::GetLayer() const
    {
        return m_config.Layer;
    }

    void UIView::SetLayer(UILayer layer)
    {
        m_config.Layer = layer;
        if (m_document)
            m_document->SetProperty("z-index", std::to_string(m_config.Layer.Order));
    }

    EUIInputMode UIView::GetInputMode() const
    {
        return m_config.InputMode;
    }

    template<typename T>
    bool UIView::BindProperty(const std::string& name, UIProperty<T>& property)
    {
        if (!m_context)
        {
            LogError("No context on '{}', can't bind data '{}'.", m_config.DocumentPath, name);
            return false;
        }

        auto model = m_context->GetDataModel(m_config.ModelName);
        if (!model)
        {
            LogError("No such model '{}' on document '{}'.", m_config.ModelName, m_config.DocumentPath);
            return false;
        }

        if (!model.Bind(name, &property.m_value))
        {
            LogError("Failed to bind data '{}' in '{}'.", name, m_config.DocumentPath);
            return false;
        }

        property.m_onChanged = [this, name] { Dirty(name); };
        m_unbinders.push_back([&property] { property.m_onChanged = nullptr; });
        return true;
    }

    bool UIView::BindData(const std::string& name, UIProperty<int>& property)
    {
        return BindProperty(name, property);
    }

    bool UIView::BindData(const std::string& name, UIProperty<float>& property)
    {
        return BindProperty(name, property);
    }

    bool UIView::BindData(const std::string& name, UIProperty<bool>& property)
    {
        return BindProperty(name, property);
    }

    bool UIView::BindData(const std::string& name, UIProperty<std::string>& property)
    {
        return BindProperty(name, property);
    }

    void UIView::Dirty(const std::string& name)
    {
        if (!m_context)
        {
            LogError("No context on '{}', can't set dirty '{}'.", m_config.DocumentPath, name);
            return;
        }

        auto model = m_context->GetDataModel(m_config.ModelName);
        if (!model)
        {
            LogError("No such model '{}' on document '{}'.", m_config.ModelName, m_config.DocumentPath);
            return;
        }

        auto handle = model.GetModelHandle();
        if (!handle)
        {
            LogError("Failed to get handle on '{}', '{}'.", m_config.ModelName, m_config.DocumentPath);
            return;
        }

        handle.DirtyVariable(name);
    }

    void UIView::DirtyAll()
    {
        if (!m_context)
        {
            LogError("No context on '{}', can't set dirty all.", m_config.DocumentPath);
            return;
        }

        auto model = m_context->GetDataModel(m_config.ModelName);
        if (!model)
        {
            LogError("No such model '{}' on document '{}'.", m_config.ModelName, m_config.DocumentPath);
            return;
        }

        auto handle = model.GetModelHandle();
        if (!handle)
        {
            LogError("Failed to get handle on '{}', '{}'.", m_config.ModelName, m_config.DocumentPath);
            return;
        }

        handle.DirtyAllVariables();
    }

    Rml::Element* UIView::Bind(const std::string& id)
    {
        Rml::Element* element = m_document ? m_document->GetElementById(id) : nullptr;
        if (!element)
        {
            LogError("Failed to bind element '{}' to '{}'.", id, m_config.DocumentPath);
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
        m_context = &context;

        if (!m_config.ModelName.empty())
        {
            if (m_context->CreateDataModel(m_config.ModelName))
            {
                OnBindModel();
            }
            else
            {
                LogError("Failed to load model '{}' in document '{}'.", m_config.ModelName, m_config.DocumentPath);
                return false;
            }
        }

        m_document = context.LoadDocument(m_config.DocumentPath);

        if (!m_document)
        {
            LogError("Failed to load view document '{}'.", m_config.DocumentPath);
            ReleaseModel();
            return false;
        }

        OnInitialize();

        m_document->SetProperty("z-index", std::to_string(m_config.Layer.Order));
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

        ReleaseModel();
        m_listeners.clear();
    }

    void UIView::ReleaseModel()
    {
        for (auto& unbind: m_unbinders)
            unbind();
        m_unbinders.clear();

        if (m_context && !m_config.ModelName.empty())
            m_context->RemoveDataModel(m_config.ModelName);
    }
} // RTGDEngine
