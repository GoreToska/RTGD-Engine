//
// Created by ivan on 6/19/26.
//

#pragma once
#include "gainput/gainput.h"

namespace RTGDEngine {
    class IInjectableButton {
    public:
        virtual ~IInjectableButton() = default;

        virtual void InjectButton(gainput::DeviceButtonId, bool) = 0;
    };

    class IInjectableAxis {
    public:
        virtual ~IInjectableAxis() = default;

        virtual void InjectAxis(gainput::DeviceButtonId, float) = 0;
    };
}
