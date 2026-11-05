//
// Created by ivan on 11/5/26.
//

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Engine/EngineExport.h"

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
        explicit UIView(const std::string& relativeDocumentPath);

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
        };

        std::string m_documentPath;
        Rml::ElementDocument* m_document = nullptr;
        std::vector<Registration> m_listeners = {};
    };
} // RTGDEditor
