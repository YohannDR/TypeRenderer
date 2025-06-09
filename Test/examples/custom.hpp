#pragma once

#include "type_renderer.hpp"

class CustomVector2
{
public:
    float_t x = 0.f;
    float_t y = 0.f;
};

// Declare a simple type renderer implementation
DEFINE_TYPE_RENDERER_TYPE(CustomVector2)

// Then define it, and render it however you want
template <typename ReflectT, typename DescriptorT, size_t Depth>
bool_t TypeRendererImpl<CustomVector2>::Render(const TypeRenderer::Metadata<ReflectT, CustomVector2, DescriptorT, Depth>& metadata)
{
    return ImGui::DragFloat2(metadata.name, &metadata.obj->x);
}

class CustomExample
{
public:
    // Since there is a TypeRendererImpl specification of CustomVector2, it can be reflected like any other type
    CustomVector2 vector;
};

REFL_AUTO(
    type(CustomExample),

    field(vector)
)

