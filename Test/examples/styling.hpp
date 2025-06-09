#pragma once

#include "reflection.hpp"

class StylingExample
{
public:
    float_t value1 = 0.f;
    float_t value2 = 0.f;
    float_t value3 = 0.f;

    void Test() const {}
};

REFL_AUTO(
    type(StylingExample),

    field(value1),
    field(value2, Reflection::PaddingX(20.f)),
    field(value3, Reflection::PaddingY(20.f)),

    func(Test, Reflection::PaddingX(30.f), Reflection::PaddingY(30.f))
);
