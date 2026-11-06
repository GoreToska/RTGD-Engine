//
// Created by ivan on 11/5/26.
//

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Engine/EngineExport.h"
#include "UI/GUIEventListener.h"

namespace Rml
{
    class Context;
    class ElementDocument;
    class Event;
    class Element;
}

namespace RTGDEngine
{
    class ENGINE_API UIView
    {
    public:
        explicit UIView(const std::string& relativeDocumentPath);

        UIView(const UIView&) = delete;

        UIView& operator=(const UIView&) = delete;

        UIView(UIView&&) noexcept = default;

        UIView& operator=(UIView&&) noexcept = default;

        virtual ~UIView();

    protected:
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

        Rml::Element* Bind(const std::string& id);

        void On(Rml::Element* element, const std::string& event, std::function<void(Rml::Event&)> callback);

        void On(const std::string& id, const std::string& event, std::function<void(Rml::Event&)> callback);

        void Close();

        Rml::ElementDocument* GetDocument() const;

    private:
        friend class UISystem;

        bool Load(Rml::Context& context);

        void Destroy();

        struct Registration
        {
            Rml::Element* Element;
            std::string Event;
            std::unique_ptr<GUIEventListener> Listener;

            Registration() = default;

            Registration(Rml::Element* inElement, std::string inEvent, std::unique_ptr<GUIEventListener> inListener)
                : Element(inElement), Event(std::move(inEvent)), Listener(std::move(inListener))
            {
            }

            Registration(Registration&&) noexcept = default;

            Registration& operator=(Registration&&) noexcept = default;

            Registration(const Registration&) = delete;

            Registration& operator=(const Registration&) = delete;
        };

        std::string m_documentPath;
        Rml::ElementDocument* m_document = nullptr;
        std::vector<Registration> m_listeners;
    };
} // RTGDEditor
