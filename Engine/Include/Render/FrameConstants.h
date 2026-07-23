//
// Created by ivan on 7/14/26.
//

#pragma once
#include "ConstBuffers.h"
#include "RenderDevice.h"

namespace RTGDEngine {
    class FrameConstants {
    public:
        void Initialize(Diligent::IRenderDevice &device, Diligent::IDeviceContext &context);

        void UpdateCamera(const CameraConstantBuffer &data) const;

        void UpdateObject(const ObjectConstantBuffer &data) const;

        void UpdateLight(const LightConstantBuffer &data) const;

        void UpdateShadow(const ShadowConstantBuffer &data) const;

        [[nodiscard]] Diligent::IBuffer &Camera() const;

        [[nodiscard]] Diligent::IBuffer &Light() const;

        [[nodiscard]] Diligent::IBuffer &Object() const;

        [[nodiscard]] Diligent::IBuffer &Shadow() const;
    private:
        Diligent::IDeviceContext *m_context = nullptr; // we dont own this, render system does
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_cameraCB;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_lightCB;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_objectCB;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_shadowCB;
    };
} // RTGD_Engine
