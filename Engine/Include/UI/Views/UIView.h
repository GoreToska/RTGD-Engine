//
// Created by ivan on 11/5/26.
//

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Engine/EngineExport.h"
#include "UI/UIProperty.h"

namespace Rml
{
    class Context;
    class ElementDocument;
    class Event;
    class Element;
}

namespace RTGDEngine
{
    class GUIEventListener;

    class ENGINE_API UIView
    {
    public:
        explicit UIView(const std::string& relativeDocumentPath, std::string modelName = {});

        UIView(const UIView&) = delete;

        UIView& operator=(const UIView&) = delete;

        UIView(UIView&&) = delete;

        UIView& operator=(UIView&&) = delete;

        virtual ~UIView();

    protected:
        virtual void OnBindModel()
        {
        }

        virtual void OnInitialize()
        {
        }

        virtual void OnShow()
        {
        }

        virtual void OnHide()
        {
        }

        virtual void OnUpdate(float deltaTime)
        {
        }

        virtual void OnDestroy()
        {
        }

        bool BindData(const std::string& name, UIProperty<int>& property);

        bool BindData(const std::string& name, UIProperty<float>& property);

        bool BindData(const std::string& name, UIProperty<bool>& property);

        bool BindData(const std::string& name, UIProperty<std::string>& property);

        void Dirty(const std::string& name);

        void DirtyAll();

        Rml::Element* Bind(const std::string& id);

        void On(Rml::Element* element, const std::string& event, std::function<void(Rml::Event&)> callback);

        void On(const std::string& id, const std::string& event, std::function<void(Rml::Event&)> callback);

        void Close();

        Rml::ElementDocument* GetDocument() const;

    private:
        friend class UISystem;

        bool Load(Rml::Context& context);

        void Destroy();

        template <typename T>
        bool BindProperty(const std::string& name, UIProperty<T>& property);

        void ReleaseModel();

        struct Registration
        {
            Rml::Element* Element;
            std::string Event;
            std::unique_ptr<GUIEventListener> Listener;
        };

        std::string m_documentPath;
        std::string m_modelName;
        Rml::ElementDocument* m_document = nullptr;
        Rml::Context* m_context = nullptr;
        std::vector<Registration> m_listeners;
        std::vector<std::function<void()>> m_unbinders;
    };
} // RTGDEngine
