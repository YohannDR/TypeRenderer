#pragma once

#include <iostream>
#include <string>

#include "imgui.h"
#include "macros.hpp"
#include "meta_programming.hpp"
#include "reflection.hpp"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "magic_enum/include/magic_enum/magic_enum.hpp"

// ReSharper disable CppClangTidyBugproneMacroParentheses

#pragma region Definition

class TypeRenderer final
{
    STATIC_CLASS(TypeRenderer)

public:
    /// @brief Metadata used to process a field
    /// @tparam ReflectT Reflected top level type
    /// @tparam MemberT Member type
    /// @tparam DescriptorT Member descriptor type
    template <typename ReflectT, typename MemberT, typename DescriptorT>
    struct Metadata
    {
        /// @brief Reflected top level object
        ReflectT* topLevelObj;

        /// @brief Member name
        const char_t* name;

        /// @brief Member object
        MemberT* obj;
    };

    /// @brief Renders a top level type with all its members
    /// @tparam ReflectT Top level type
    /// @param obj Object
    /// @return bool_t, whether any member has been modified
    template <typename ReflectT>
    static bool_t RenderType(ReflectT* obj);

    /// @brief Displays a simple member
    /// @tparam ReflectT Reflected top level type
    /// @tparam MemberT Member type
    /// @tparam DescriptorT Field descriptor type
    /// @param metadata Member metadata
    /// @return bool_t, whether the member changed has been modified
    template <typename ReflectT, typename MemberT, typename DescriptorT>
    static bool_t DisplaySimpleMember(const Metadata<ReflectT, MemberT, DescriptorT>& metadata);
    
private:
    template <typename ReflectT, bool_t IsStatic>
    static bool_t DisplayMembers(ReflectT* obj);

    template <typename ReflectT, typename DescriptorT, bool_t IsStatic>
    static bool_t DisplayField(ReflectT* obj, bool_t& hasStatic);

    template <typename ReflectT, typename DescriptorT>
    static void DisplayFunction(ReflectT* obj);

    template <typename ReflectT, typename MemberT, typename DescriptorT>
    static bool_t DisplayObjectInternal(ReflectT* obj);

    template <typename ReflectT, typename MemberT, typename DescriptorT>
    static void CheckDisplayTooltip(const Metadata<ReflectT, MemberT, DescriptorT>& metadata);
};

/// @brief Implementation for a type renderer, template specialization can be used to provide a custom render behavior to a custom type
/// @tparam MemberT Member type
/// @tparam Condition Conditional template to use enable if 
template <typename MemberT, typename Condition = void>
struct TypeRendererImpl
{
    /// @brief Renders the provided type
    /// @tparam ReflectT Reflected top level type
    /// @tparam DescriptorT Field descriptor type
    /// @param metadata Member metadata
    template <typename ReflectT, typename DescriptorT>
    static bool_t Render(const TypeRenderer::Metadata<ReflectT, MemberT, DescriptorT>& metadata);
};

#define DEFINE_TYPE_RENDERER_COND(condition)                                                        \
template <typename MemberT>                                                                         \
struct TypeRendererImpl<MemberT, Meta::EnableIf<condition>>                                         \
{                                                                                                   \
    template <typename ReflectT, typename DescriptorT>                                              \
    static bool_t Render(const TypeRenderer::Metadata<ReflectT, MemberT, DescriptorT>& metadata);   \
};                                                                                                  \

#define DEFINE_TYPE_RENDERER_TYPE(type)                                                         \
template <>                                                                                     \
struct TypeRendererImpl<type>                                                                   \
{                                                                                               \
    template <typename ReflectT, typename DescriptorT>                                          \
    static bool_t Render(const TypeRenderer::Metadata<ReflectT, type, DescriptorT>& metadata);  \
};    

DEFINE_TYPE_RENDERER_COND(Meta::IsIntegralNumericOrFloating<MemberT>)
DEFINE_TYPE_RENDERER_COND(Meta::IsEnum<MemberT>)

DEFINE_TYPE_RENDERER_TYPE(bool_t)
DEFINE_TYPE_RENDERER_TYPE(std::string)

#pragma endregion

#pragma region Implementation

template <typename ReflectT>
bool_t TypeRenderer::RenderType(ReflectT* const obj)
{
    return DisplayMembers<ReflectT, false>(obj) || DisplayMembers<ReflectT, true>(obj);
}

template <typename ReflectT, bool_t IsStatic>
bool_t TypeRenderer::DisplayMembers(ReflectT* const obj)
{
    // Get reflected data
    constexpr TypeDescriptor<ReflectT> desc = Reflection::GetTypeInfo<ReflectT>();

    bool_t hasStatic = false;
    bool_t anyChanged = false;

    // Loop over each reflected member
    refl::util::for_each(desc.members, [&]<typename DescriptorT>(const DescriptorT)
    {
        if constexpr (Reflection::IsFunction<DescriptorT>)
        {
            if constexpr (!IsStatic)
                DisplayFunction<ReflectT, DescriptorT>(obj);
        }
        else
        {
            anyChanged |= DisplayField<ReflectT, DescriptorT, IsStatic>(obj, hasStatic);
        }
    });

    if constexpr (IsStatic)
    {
        if (hasStatic)
        {
            ImGui::Separator();
        }
    }

    return anyChanged;
}

template <typename ReflectT, typename DescriptorT, bool_t IsStatic>
bool_t TypeRenderer::DisplayField(ReflectT* const obj, bool_t& hasStatic)
{
    // Get member type
    using MemberT = Meta::RemoveConstSpecifier<typename DescriptorT::value_type>;

    // Shorthand for the templated attributes
    using NotifyChangeT = Reflection::NotifyChange<ReflectT>;
    using ModifiedCallbackT = Reflection::ModifiedCallback<ReflectT>;

    constexpr bool_t isConst = !DescriptorT::is_writable;
    constexpr bool_t readOnly = Reflection::HasAttribute<Reflection::ReadOnly, DescriptorT>();

    // We want to display static fields when IsStatic is true, and member fields when IsStatic is false
    // Hence the binary operations
    // Truth table :
    // IsStatic | is_static | !is_static | Result
    // 0        | 0         | 1          | 1
    // 0        | 1         | 0          | 0
    // 1        | 0         | 1          | 1
    // 1        | 1         | 0          | 0
    constexpr bool_t display = IsStatic ^ !DescriptorT::is_static;

    if constexpr (IsStatic && DescriptorT::is_static)
    {
        hasStatic = true;
    }

    bool_t changed = false;
    if constexpr (display)
    {
        ImGui::BeginDisabled(isConst || readOnly);

        changed = DisplayObjectInternal<ReflectT, MemberT, DescriptorT>(obj);

        if (changed)
        {
            if constexpr (Reflection::HasAttribute<NotifyChangeT, DescriptorT>())
            {
                // Value was changed, set the pointer to true
                constexpr NotifyChangeT notify = Reflection::GetAttribute<NotifyChangeT, DescriptorT>();
                obj->*notify.pointer = true;
            }
            else if constexpr (Reflection::HasAttribute<ModifiedCallbackT, DescriptorT>())
            {
                constexpr ModifiedCallbackT notify = Reflection::GetAttribute<ModifiedCallbackT, DescriptorT>();
                notify.callback(obj);
            }
        }

        ImGui::EndDisabled();
    }

    return changed;
}

template <typename ReflectT, typename DescriptorT>
void TypeRenderer::DisplayFunction(ReflectT* const obj)
{
    // Get variable name
    constexpr const char_t* const name = DescriptorT::name.c_str();

    using MemberT = decltype(DescriptorT::pointer);

    // Construct metadata
    const Metadata<ReflectT, MemberT, DescriptorT> metadata = {
        .topLevelObj = obj,
        .name = name,
        .obj = &DescriptorT::pointer
    };

    if (ImGui::Button(DescriptorT::name.c_str()))
    {
        (metadata.topLevelObj->*(*metadata.obj))();
    }

    CheckDisplayTooltip(metadata);
}

template <typename ReflectT, typename MemberT, typename DescriptorT>
bool_t TypeRenderer::DisplayObjectInternal(ReflectT* const obj)
{
    // Get variable name
    constexpr const char_t* const name = DescriptorT::name.c_str();

    // Construct metadata
    const Metadata<ReflectT, MemberT, DescriptorT> metadata = {
        .topLevelObj = obj,
        .name = name,
        .obj = [&]() -> MemberT*
        {
            if constexpr (DescriptorT::is_static)
                return const_cast<MemberT*>(&DescriptorT::get());
            else
                return const_cast<MemberT*>(&DescriptorT::get(*obj));
        }()
    };

    ImGui::PushID(metadata.obj);

    const bool_t changed = DisplaySimpleMember<ReflectT, MemberT, DescriptorT>(metadata);

    CheckDisplayTooltip(metadata);

    ImGui::PopID();

    return changed;
}

template <typename ReflectT, typename MemberT, typename DescriptorT>
void TypeRenderer::CheckDisplayTooltip(const Metadata<ReflectT, MemberT, DescriptorT>& metadata)
{
    using DynamicTooltip = Reflection::DynamicTooltip<ReflectT>;
    
    // Check for a tooltip
    if constexpr (Reflection::HasAttribute<Reflection::Tooltip, DescriptorT>())
    {
        // Set tooltip
        ImGui::SetItemTooltip("%s", Reflection::GetAttribute<Reflection::Tooltip, DescriptorT>().text);
    }
    else if constexpr (Reflection::HasAttribute<DynamicTooltip, DescriptorT>())
    {
        constexpr DynamicTooltip tooltip = Reflection::GetAttribute<DynamicTooltip, DescriptorT>();
        if (!(metadata.topLevelObj->*tooltip.text).empty())
        {
            // Set tooltip
            ImGui::SetItemTooltip("%s", (metadata.topLevelObj->*tooltip.text).c_str());
        }
    }
}

template <typename ReflectT, typename MemberT, typename DescriptorT>
bool_t TypeRenderer::DisplaySimpleMember(const Metadata<ReflectT, MemberT, DescriptorT>& metadata)
{
    return TypeRendererImpl<MemberT>::template Render<ReflectT, DescriptorT>(metadata);
}

#pragma endregion

#pragma region Type implementation

template <typename MemberT, typename Condition>
template <typename ReflectT, typename DescriptorT>
bool_t TypeRendererImpl<MemberT, Condition>::Render(const TypeRenderer::Metadata<ReflectT, MemberT, DescriptorT>& metadata)
{
    if constexpr (Reflection::IsReflected<MemberT>)
    {
        return TypeRenderer::RenderType<MemberT>(metadata.obj);
    }
    else
    {
        std::cout << "Type doesn't have a TypeRenderImpl : " << metadata.name << " of type " << typeid(MemberT).name() << std::endl;
        return false;
    }
}

template <typename MemberT>
template <typename ReflectT, typename DescriptorT>
bool_t TypeRendererImpl<MemberT, Meta::EnableIf<Meta::IsIntegralNumericOrFloating<MemberT>>>::Render(const TypeRenderer::Metadata<ReflectT, MemberT, DescriptorT>& metadata)
{
    uint32_t type;

    // Get imgui data type based on the member type
    if constexpr (Meta::IsSame<MemberT, int64_t>)
        type = ImGuiDataType_S64;
    else if constexpr (Meta::IsSame<MemberT, uint64_t>)
        type = ImGuiDataType_U64;
    else if constexpr (Meta::IsSame<MemberT, int32_t>)
        type = ImGuiDataType_S32;
    else if constexpr (Meta::IsSame<MemberT, uint32_t>)
        type = ImGuiDataType_U32;
    else if constexpr (Meta::IsSame<MemberT, int16_t>)
        type = ImGuiDataType_S16;
    else if constexpr (Meta::IsSame<MemberT, uint16_t>)
        type = ImGuiDataType_U16;
    else if constexpr (Meta::IsSame<MemberT, int8_t>)
        type = ImGuiDataType_S8;
    else if constexpr (Meta::IsSame<MemberT, uint8_t>)
        type = ImGuiDataType_U8;
    else if constexpr (Meta::IsSame<MemberT, float_t>)
        type = ImGuiDataType_Float;
    else if constexpr (Meta::IsSame<MemberT, double_t>)
        type = ImGuiDataType_Double;

    using Range = Reflection::Range<MemberT>;
    using DynamicRange = Reflection::DynamicRange<ReflectT, MemberT>;

    if constexpr (Reflection::HasAttribute<Range, DescriptorT>())
    {
        constexpr Range range = Reflection::GetAttribute<Range, DescriptorT>();
        return ImGui::SliderScalar(metadata.name, type, metadata.obj, &range.minimum, &range.maximum);
    }
    else if constexpr (Reflection::HasAttribute<DynamicRange, DescriptorT>())
    {
        constexpr DynamicRange range = Reflection::GetAttribute<DynamicRange, DescriptorT>();

        if constexpr (range.minimum == nullptr)
        {
            constexpr MemberT zero = 0;
            const MemberT max = metadata.topLevelObj->*range.maximum;
            return ImGui::SliderScalar(metadata.name, type, metadata.obj, &zero, &max);    
        }
        else
        {
            const MemberT min = metadata.topLevelObj->*range.minimum;
            const MemberT max = metadata.topLevelObj->*range.maximum;
            return ImGui::SliderScalar(metadata.name, type, metadata.obj, &min, &max);
        }
    }
    else
    {
        return ImGui::DragScalar(metadata.name, type, metadata.obj, .1f);
    }
}

template <typename ReflectT, typename DescriptorT>
bool_t TypeRendererImpl<bool_t>::Render(const TypeRenderer::Metadata<ReflectT, bool_t, DescriptorT>& metadata)
{
    return ImGui::Checkbox(metadata.name, metadata.obj);
}

template <typename ReflectT, typename DescriptorT>
bool_t TypeRendererImpl<std::string>::Render(const TypeRenderer::Metadata<ReflectT, std::string, DescriptorT>& metadata)
{
    return ImGui::InputText(metadata.name, metadata.obj);
}

template <typename MemberT>
template <typename ReflectT, typename DescriptorT>
bool_t TypeRendererImpl<MemberT, Meta::EnableIf<Meta::IsEnum<MemberT>>>::Render(const TypeRenderer::Metadata<ReflectT, MemberT, DescriptorT>& metadata)
{
    // Get an array of the enum names
    constexpr auto enumNames = magic_enum::enum_names<MemberT>();
    // Get the type, we can't know before compile time the type because the size of the array is determined when the function is processed
    using NamesArrayT = decltype(enumNames);

    // Getter for imgui
    constexpr auto getter = [](void* const userData, const int32_t idx) -> const char_t*
    {
        const auto ptr = static_cast<NamesArrayT*>(userData);
        return ptr->at(idx).data();
    };

    void* const names = reinterpret_cast<void*>(const_cast<Meta::RemoveConstSpecifier<NamesArrayT>*>(&enumNames));
    return ImGui::Combo(metadata.name, reinterpret_cast<int32_t*>(metadata.obj), getter, names, static_cast<int32_t>(enumNames.size()));
}

#pragma endregion
