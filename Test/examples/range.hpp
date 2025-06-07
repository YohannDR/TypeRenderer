#pragma once

#include "reflection.hpp"

/// @brief Example class for the ranges attributes
class RangeExample
{
public:
    int32_t valueClamped = 10;

    int32_t min = -10;
    int32_t max = 10;
    int32_t valueDynamicallyClamped = 0;
};

REFL_AUTO(
    type(RangeExample),

    field(valueClamped, Reflection::Range(0, 20)),
    field(min),
    field(max),
    field(valueDynamicallyClamped, Reflection::DynamicRange(&RangeExample::min, &RangeExample::max))
)
