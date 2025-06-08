#pragma once

#include "reflection.hpp"

class TooltipExample
{
public:
    int32_t value = 0;

    std::string description = "I am a description";
    int32_t customTooltip = 0;
};

REFL_AUTO(
    type(TooltipExample),

    field(value, Reflection::Tooltip("This number is very important")),
    field(description, Reflection::Tooltip("Change this to modify the tooltip of the element below")),
    field(customTooltip, Reflection::DynamicTooltip(&TooltipExample::description))
);
