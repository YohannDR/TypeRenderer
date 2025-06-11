#pragma once

#include "reflection.hpp"

class StylingExample
{
public:
    float_t value1 = 0.f;
    float_t value2 = 0.f;
    float_t value3 = 0.f;
    std::string text = "Colored text";
    float_t styled = 1.f;

    void Test() const {}
};

REFL_AUTO(
    type(StylingExample),

    field(value1),
    field(value2, Reflection::PaddingX(20.f)),
    field(value3, Reflection::PaddingY(20.f)),
    field(text, Reflection::StyleColor(
        std::make_pair(ImGuiCol_Text, IM_COL32(0xFF, 0, 0, 0xFF)),
        std::make_pair(ImGuiCol_TextSelectedBg, IM_COL32(0, 0xFF, 0, 0xFF))
    )),
    field(styled,
        Reflection::StyleVar<float_t>(std::make_pair(ImGuiStyleVar_Alpha, .5f)),
        Reflection::StyleVar<ImVec2>(std::make_pair(ImGuiStyleVar_FramePadding, ImVec2(10.f, 10.f)))
    ),

    func(Test, Reflection::PaddingX(30.f), Reflection::PaddingY(30.f))
);
