//
// Created by ivan on 11/10/26.
//

#pragma once

#include <compare>

namespace RTGDEngine
{
    struct UILayer
    {
        int Order = 0;

        constexpr explicit UILayer(int order) : Order(order)
        {
        }

        constexpr auto operator<=>(const UILayer& rhs) const = default;
    };

    namespace UILayers
    {
        inline constexpr UILayer HUD{0};
        inline constexpr UILayer Menu{100};
        inline constexpr UILayer Popup{200};
        inline constexpr UILayer Debug{300};
    }
}
