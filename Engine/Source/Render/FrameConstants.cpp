//
// Created by ivan on 7/14/26.
//

#include "Render/FrameConstants.h"

#include "Tools/Logger.h"

namespace RTGDEngine {
    void FrameConstants::Initialize(Diligent::IRenderDevice &device, Diligent::IDeviceContext &context) {
        using namespace Diligent;

        m_context = &context;

        BufferDesc cbDesc;
        cbDesc.Usage = USAGE_DYNAMIC;
        cbDesc.BindFlags = BIND_UNIFORM_BUFFER;
        cbDesc.CPUAccessFlags = CPU_ACCESS_WRITE;

        cbDesc.Name = "Camera CB";
        cbDesc.Size = sizeof(CameraConstantBuffer);
        device.CreateBuffer(cbDesc, nullptr, &m_cameraCB);

        cbDesc.Name = "Object CB";
        cbDesc.Size = sizeof(ObjectConstantBuffer);
        device.CreateBuffer(cbDesc, nullptr, &m_objectCB);

        cbDesc.Name = "Light CB";
        cbDesc.Size = sizeof(LightConstantBuffer);
        device.CreateBuffer(cbDesc, nullptr, &m_lightCB);

        CameraConstantBuffer defaultCam{};
        defaultCam.View = Matrix4::Identity();
        defaultCam.Projection = Matrix4::Identity();
        UpdateCamera(defaultCam);

        ObjectConstantBuffer defaultObj{};
        defaultObj.Model = Matrix4::Identity();
        UpdateObject(defaultObj);

        LightConstantBuffer defaultLight{};
        UpdateLight(defaultLight);

        LogInfo("Constant buffers initialized");
    }

    void FrameConstants::UpdateCamera(const CameraConstantBuffer &data) const {
        using namespace Diligent;

        void *pMapped = nullptr;
        m_context->MapBuffer(m_cameraCB, MAP_WRITE, MAP_FLAG_DISCARD, pMapped);
        if (pMapped) {
            auto *dst = static_cast<CameraConstantBuffer *>(pMapped);
            dst->View = data.View.Transpose();
            dst->Projection = data.Projection.Transpose();
            dst->CameraPosition = data.CameraPosition;
            m_context->UnmapBuffer(m_cameraCB, MAP_WRITE);
        }
    }

    void FrameConstants::UpdateObject(const ObjectConstantBuffer &data) const {
        using namespace Diligent;

        void *pMapped = nullptr;
        m_context->MapBuffer(m_objectCB, MAP_WRITE, MAP_FLAG_DISCARD, pMapped);
        if (pMapped) {
            auto *dst = static_cast<ObjectConstantBuffer *>(pMapped);
            dst->Model = data.Model.Transpose();

#ifdef RTGD_EDITOR
            dst->EntityID = data.EntityID;
#endif

            m_context->UnmapBuffer(m_objectCB, MAP_WRITE);
        }
    }

    void FrameConstants::UpdateLight(const LightConstantBuffer &data) const {
        using namespace Diligent;

        void *pMapped = nullptr;
        m_context->MapBuffer(m_lightCB, MAP_WRITE, MAP_FLAG_DISCARD, pMapped);
        if (pMapped) {
            memcpy(pMapped, &data, sizeof(LightConstantBuffer));
            m_context->UnmapBuffer(m_lightCB, MAP_WRITE);
        }
    }

    Diligent::IBuffer &FrameConstants::Camera() const {
        return *m_cameraCB;
    }

    Diligent::IBuffer &FrameConstants::Light() const {
        return *m_lightCB;
    }

    Diligent::IBuffer &FrameConstants::Object() const {
        return *m_objectCB;
    }
} // RTGD_Engine
