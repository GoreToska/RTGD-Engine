//
// Created by ivan on 10/12/26.
//

#pragma once

#include <RmlUi/Core/RenderInterface.h>

#include "../../../../ThirdParty/RmlUi/Include/RmlUi/Core/RenderInterface.h"
#include "../../../../ThirdParty/RmlUi/Include/RmlUi/Core/Types.h"
#include "../../../../ThirdParty/RmlUi/Include/RmlUi/Core/Vertex.h"


namespace RTGDEngine
{
    class UiRenderInterfaceStub : public Rml::RenderInterface
    {
    public:
        Rml::CompiledGeometryHandle
        CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override
        {
            return ++m_next;
        };

        void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
                            Rml::TextureHandle texture) override
        {
        };

        void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override
        {
        };

        Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override
        {
            return ++m_next;
        };

        Rml::TextureHandle
        GenerateTexture(Rml::Span<const unsigned char> source, Rml::Vector2i source_dimensions) override
        {
            return ++m_next;
        }

        void ReleaseTexture(Rml::TextureHandle texture) override
        {
        };

        void EnableScissorRegion(bool enable) override
        {
        };

        void SetScissorRegion(Rml::Rectanglei region) override
        {
        };

        uintptr_t m_next = 0;
    };
}
