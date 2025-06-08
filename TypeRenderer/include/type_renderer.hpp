#pragma once

#include <iostream>
#include <stack>
#include <string>

#include "imgui.h"
#include "imgui_internal.h"
#include "macros.hpp"
#include "meta_programming.hpp"
#include "reflection.hpp"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "magic_enum/include/magic_enum/magic_enum.hpp"
#include "magic_enum/include/magic_enum/magic_enum_flags.hpp"

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
    /// @param inWindow Whether to render the type in a specific window, or in the current context
    /// @return bool_t, whether any member has been modified
    template <typename ReflectT>
    static bool_t RenderType(ReflectT* obj, bool_t inWindow = false);

    /// @brief Displays a simple member
    /// @tparam ReflectT Reflected top level type
    /// @tparam MemberT Member type
    /// @tparam DescriptorT Field descriptor type
    /// @param metadata Member metadata
    /// @return bool_t, whether the member changed has been modified
    template <typename ReflectT, typename MemberT, typename DescriptorT>
    static bool_t DisplaySimpleMember(const Metadata<ReflectT, MemberT, DescriptorT>& metadata);

private:
    template <typename ReflectT, typename MemberT, typename DescriptorT>
    static Metadata<ReflectT, MemberT, DescriptorT> CreateMetadata(ReflectT* obj);

    template <typename DescriptorT>
    static constexpr const char_t* GetMemberName();

    template <typename ReflectT, typename MemberT, typename DescriptorT>
    static MemberT* GetMemberPointer(ReflectT* obj);
    
    template <typename ReflectT, bool_t IsStatic>
    static bool_t DisplayMembers(ReflectT* obj);

    template <typename ReflectT, typename MemberT, typename DescriptorT, bool_t IsStatic>
    static bool_t DisplayField(const Metadata<ReflectT, MemberT, DescriptorT>& metadata, bool_t& hasStatic);

    template <typename ReflectT, typename MemberT, typename DescriptorT>
    static void DisplayFunction(const Metadata<ReflectT, MemberT, DescriptorT>& metadata);

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

enum class ItDefFlags
{
    // No flags
    None = 0,
    // Indicates that the specialization is valid 
    Exists = 1 << 0,
    // Indicates that the container can be mutated, i.e. values can be added/removed
    Mutable = 1 << 1,
    // Indicates that the container is a key value pair type
    KeyValuePair = 1 << 2,
};

constexpr ItDefFlags operator|(const ItDefFlags left, const ItDefFlags right)
{
    return static_cast<ItDefFlags>(static_cast<size_t>(left) | static_cast<size_t>(right));
}

constexpr size_t operator&(const ItDefFlags left, const ItDefFlags right)
{
    return static_cast<size_t>(left) & static_cast<size_t>(right);
}

template <typename>
struct ContainerDefinition
{
    static constexpr ItDefFlags Flags = ItDefFlags::None;
};

template <typename T, size_t N>
struct ContainerDefinition<T[N]>
{
    static constexpr ItDefFlags Flags = ItDefFlags::Exists;

    using Type = T[N];
    using PtrType = T (*)[N];
    using KeyType = decltype(nullptr);
    using ValueType = T;

    static constexpr size_t GetSize(const PtrType) { return N; }
    static ValueType& GetElement(PtrType array, const size_t index) { return (*array)[index]; }
};

template <typename T, size_t N>
struct ContainerDefinition<std::array<T, N>>
{
    static constexpr ItDefFlags Flags = ItDefFlags::Exists;

    using Type = std::array<T, N>;
    using PtrType = std::array<T, N>*;
    using KeyType = decltype(nullptr);
    using ValueType = T;

    static constexpr size_t GetSize(const PtrType) { return N; }
    static ValueType& GetElement(PtrType array, const size_t index) { return (*array)[index]; }
};

template <typename T>
struct ContainerDefinition<std::vector<T>>
{
    static constexpr ItDefFlags Flags = ItDefFlags::Exists | ItDefFlags::Mutable;

    using Type = std::vector<T>;
    using PtrType = std::vector<T>*;
    using KeyType = decltype(nullptr);
    using ValueType = T;

    static size_t GetSize(const PtrType array) { return array->size(); }
    static ValueType& GetElement(PtrType array, const size_t index) { return (*array)[index]; }
    static void AddElement(PtrType array) { array->emplace_back(); }
    static void Insert(PtrType array, const size_t index) { array->insert(array->begin() + index, ValueType()); }
    static void Remove(PtrType array, const size_t index) { array->erase(array->begin() + index); }
};

template <typename K, typename T>
struct ContainerDefinition<std::unordered_map<K, T>>
{
    static constexpr ItDefFlags Flags = ItDefFlags::Exists | ItDefFlags::Mutable | ItDefFlags::KeyValuePair;

    using Type = std::unordered_map<K, T>;
    using PtrType = std::unordered_map<K, T>*;
    using KeyType = K;
    using ValueType = T;

    static void AddElement(PtrType array) { array->insert({KeyType(), ValueType()}); }
    static void ChangeElement(PtrType array, const KeyType& key, const ValueType& value) { (*array)[key] = value; }
    static void Remove(PtrType array, const KeyType& key) { array->erase(key); }
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
DEFINE_TYPE_RENDERER_COND(ContainerDefinition<MemberT>::Flags & ItDefFlags::Exists && !(ContainerDefinition<MemberT>::Flags & ItDefFlags::KeyValuePair))
DEFINE_TYPE_RENDERER_COND(ContainerDefinition<MemberT>::Flags & ItDefFlags::Exists && ContainerDefinition<MemberT>::Flags & ItDefFlags::KeyValuePair)

DEFINE_TYPE_RENDERER_TYPE(bool_t)
DEFINE_TYPE_RENDERER_TYPE(std::string)

#pragma endregion

#pragma region Implementation

template <typename ReflectT>
bool_t TypeRenderer::RenderType(ReflectT* const obj, const bool_t inWindow)
{
    if (inWindow)
    {
        if (!ImGui::Begin(Reflection::GetTypeInfo<ReflectT>().name.c_str()))
        {
            ImGui::End();
            return false;
        }
    }
    
    const bool_t changed = DisplayMembers<ReflectT, false>(obj) || DisplayMembers<ReflectT, true>(obj);

    if (inWindow)
        ImGui::End();
    return changed;
}

template <typename ReflectT, typename MemberT, typename DescriptorT>
TypeRenderer::Metadata<ReflectT, MemberT, DescriptorT> TypeRenderer::CreateMetadata(ReflectT* const obj)
{
    const Metadata<ReflectT, MemberT, DescriptorT> metadata = {
        .topLevelObj = obj,
        .name = GetMemberName<DescriptorT>(),
        .obj = GetMemberPointer<ReflectT, MemberT, DescriptorT>(obj)
    };

    return metadata;
}

template <typename DescriptorT>
constexpr const char_t* TypeRenderer::GetMemberName()
{
    if constexpr (Reflection::HasAttribute<Reflection::CustomName, DescriptorT>())
        return Reflection::GetAttribute<Reflection::CustomName, DescriptorT>().text;

    return DescriptorT::name.c_str();
}

template <typename ReflectT, typename MemberT, typename DescriptorT>
MemberT* TypeRenderer::GetMemberPointer(MAYBE_UNUSED ReflectT* const obj)
{
    if constexpr (Reflection::IsFunction<DescriptorT>)
        return &DescriptorT::pointer;
    else if constexpr (DescriptorT::is_static)
        return const_cast<MemberT*>(&DescriptorT::get());
    else
        return const_cast<MemberT*>(&DescriptorT::get(*obj));
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
        using MemberT = Reflection::GetMemberT<DescriptorT>;

        const Metadata<ReflectT, MemberT, DescriptorT> metadata = CreateMetadata<ReflectT, MemberT, DescriptorT>(obj);
        
        if constexpr (Reflection::IsFunction<DescriptorT>)
        {
            if constexpr (!IsStatic)
                DisplayFunction<ReflectT, MemberT, DescriptorT>(metadata);
        }
        else
        {
            anyChanged |= DisplayField<ReflectT, MemberT, DescriptorT, IsStatic>(metadata, hasStatic);
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

template <typename ReflectT, typename MemberT, typename DescriptorT, bool_t IsStatic>
bool_t TypeRenderer::DisplayField(const Metadata<ReflectT, MemberT, DescriptorT>& metadata, bool_t& hasStatic)
{
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
        ImGui::PushID(metadata.obj);
        changed = DisplaySimpleMember<ReflectT, MemberT, DescriptorT>(metadata);
        CheckDisplayTooltip(metadata);
        ImGui::PopID();

        if (changed)
        {
            if constexpr (Reflection::HasAttribute<NotifyChangeT, DescriptorT>())
            {
                // Value was changed, set the pointer to true
                constexpr NotifyChangeT notify = Reflection::GetAttribute<NotifyChangeT, DescriptorT>();
                metadata.topLevelObj->*notify.pointer = true;
            }
            else if constexpr (Reflection::HasAttribute<ModifiedCallbackT, DescriptorT>())
            {
                constexpr ModifiedCallbackT notify = Reflection::GetAttribute<ModifiedCallbackT, DescriptorT>();
                notify.callback(metadata.topLevelObj);
            }
        }

        ImGui::EndDisabled();
    }

    return changed;
}

template <typename ReflectT, typename MemberT, typename DescriptorT>
void TypeRenderer::DisplayFunction(const Metadata<ReflectT, MemberT, DescriptorT>& metadata)
{
    if (ImGui::Button(metadata.name))
    {
        (metadata.topLevelObj->*(*metadata.obj))();
    }

    CheckDisplayTooltip(metadata);
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
    ImGui::PushID(metadata.obj);
    const bool_t changed = TypeRendererImpl<MemberT>::template Render<ReflectT, DescriptorT>(metadata);
    ImGui::PopID();
    return changed;
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

template <typename MemberT>
template <typename ReflectT, typename DescriptorT>
bool_t TypeRendererImpl<MemberT, Meta::EnableIf<ContainerDefinition<MemberT>::Flags & ItDefFlags::Exists && !(ContainerDefinition<MemberT>::Flags & ItDefFlags::KeyValuePair)>>::Render(const TypeRenderer::Metadata<ReflectT, MemberT, DescriptorT>& metadata)
{
    using ItDef = ContainerDefinition<MemberT>;
    using ValueType = typename ItDef::ValueType;

    constexpr bool_t isMutable = ItDef::Flags & ItDefFlags::Mutable;

    if (!ImGui::CollapsingHeader(metadata.name))
        return false;

    using AddCallback = Reflection::ContainerAddCallback<ReflectT, ValueType>;
    constexpr bool_t hasAddCallback = Reflection::HasAttribute<AddCallback, DescriptorT>();
    
    bool_t changed = false;
    if constexpr (isMutable)
    {
        if (ImGui::Button("Add element"))
        {
            ItDef::AddElement(metadata.obj);
            changed = true;

            if constexpr (hasAddCallback)
            {
                ValueType& last = ItDef::GetElement(metadata.obj, ItDef::GetSize(metadata.obj) - 1);
                (metadata.topLevelObj->*Reflection::GetAttribute<AddCallback, DescriptorT>().func)(last);
            }
        }
    }

    size_t listSize = ItDef::GetSize(metadata.obj);
    for (size_t i = 0; i < listSize; i++)
    {
        ValueType& v = ItDef::GetElement(metadata.obj, i);

        if constexpr (isMutable)
        {
            ImGui::PushID(&i + i);
            // Button to delete the current element
            if (ImGui::Button("-"))
            {
                ItDef::Remove(metadata.obj, i);
                --listSize;
                changed = true;

                // Check for special cases :
                // - The element we deleted was the only one in the list, so the list is now empty
                // - The element we deleted was the last one in the list, so we don't have anything else to process
                if (listSize == 0 || i == listSize)
                {
                    ImGui::PopID();
                    break;
                }
            }

            ImGui::SameLine();

            // Handle inserting elements
            if (ImGui::Button("+"))
            {
                // Insert a new element at the given position
                ItDef::Insert(metadata.obj, i);
                ++listSize;
                changed = true;

                if constexpr (hasAddCallback)
                {
                    (metadata.topLevelObj->*Reflection::GetAttribute<AddCallback, DescriptorT>().func)(ItDef::GetElement(metadata.obj, i));
                }
            }
                
            ImGui::PopID();
            ImGui::SameLine();
        }

        const std::string name = std::to_string(i);
        const TypeRenderer::Metadata<ReflectT, ValueType, DescriptorT> elementMetadata = {
            .topLevelObj = metadata.topLevelObj,
            .name = name.c_str(),
            .obj = &v,
        };

        changed |= TypeRenderer::DisplaySimpleMember(elementMetadata);
    }

    return changed;
}

template <typename MemberT>
template <typename ReflectT, typename DescriptorT>
bool_t TypeRendererImpl<MemberT, Meta::EnableIf<ContainerDefinition<MemberT>::Flags & ItDefFlags::Exists && ContainerDefinition<MemberT>::Flags & ItDefFlags::KeyValuePair>>::Render(const TypeRenderer::Metadata<ReflectT, MemberT, DescriptorT>& metadata)
{
    using ItDef = ContainerDefinition<MemberT>;
    using KeyType = typename ItDef::KeyType;
    using ValueType = typename ItDef::ValueType;

    if (!ImGui::CollapsingHeader(metadata.name))
        return false;
    
    bool_t changed = false;

    if (ImGui::Button("Add element"))
    {
        ItDef::AddElement(metadata.obj);
        changed = true;
    }

    // TODO find a solution to items being reordered upon re-hashing, which causes funky stuff with ImGui
    
    KeyType changeOldKey;
    KeyType changeNewKey;
    ValueType changeValue;
    bool_t requireChange = false;
    size_t i = 0;
    for (std::pair<KeyType, ValueType> pair : *metadata.obj)
    {
        ImGui::PushID(&i + i);
        // Button to delete the current element
        if (ImGui::Button("-"))
        {
            ItDef::Remove(metadata.obj, pair.first);
            changed = true;
            ImGui::PopID();
            break;
        }

        ImGui::PopID();
        ImGui::SameLine();

        const KeyType oldKey = pair.first;
        const ValueType oldValue = pair.second;
        KeyType keyDupe = oldKey;
        ValueType valueDupe = oldValue;

        const std::string name = std::to_string(i);
        const TypeRenderer::Metadata<ReflectT, KeyType, DescriptorT> keyMetadata = {
            .topLevelObj = metadata.topLevelObj,
            .name = "",
            .obj = &keyDupe,
        };

        const TypeRenderer::Metadata<ReflectT, ValueType, DescriptorT> valueMetadata = {
            .topLevelObj = metadata.topLevelObj,
            .name = name.c_str(),
            .obj = &valueDupe,
        };

        ImGui::PushID(keyMetadata.obj + i);
        ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .45f);
        const bool_t keyChanged = TypeRenderer::DisplaySimpleMember(keyMetadata);
        ImGui::PopID();

        changed |= keyChanged;

        if (keyChanged)
        {
            changeOldKey = oldKey;
            changeNewKey = keyDupe;
            changeValue = oldValue;
            requireChange = true;
        }

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        ImGui::PushID(valueMetadata.obj + i);
        ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .45f);
        const bool_t valueChanged = TypeRenderer::DisplaySimpleMember(valueMetadata);
        ImGui::PopID();

        changed |= valueChanged;

        if (valueChanged)
        {
            ItDef::ChangeElement(metadata.obj, keyDupe, valueDupe);
        }
        i++;
    }

    if (requireChange)
    {
        std::cout << "Changing from " << changeOldKey << " to " << changeNewKey << std::endl;
        ItDef::Remove(metadata.obj, changeOldKey);
        ItDef::ChangeElement(metadata.obj, changeNewKey, changeValue);
    }

    return changed;
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

    if constexpr (Reflection::HasAttribute<Reflection::EnumFlags, DescriptorT>())
    {
        constexpr size_t size = enumNames.size();

        const MemberT oldValue = *metadata.obj;

        // Compute preview value, it should display as all the selected values, comma separated
        // e.g. : Value1,Value2,Value8
        auto v = magic_enum::enum_flags_name<MemberT>(*metadata.obj, ',');
        const char_t* previewValue;
        if (static_cast<size_t>(*metadata.obj) == 0)
            previewValue = "<None>";
        else
            previewValue = v.data();

        // Need to do a custom combo implementation because we can select multiple values
        if (ImGui::BeginCombo(metadata.name, previewValue))
        {
            for (size_t i = 0; i < size; i++)
            {
                // Get enum value
                const size_t enumValue = static_cast<size_t>(magic_enum::enum_value<MemberT>(i));
                // Get member value
                size_t value = static_cast<size_t>(*metadata.obj);
                // Check if the enum value is set
                const bool_t isSelected = (enumValue != 0) && (value & enumValue) == enumValue;

                if (ImGui::MenuItem(enumNames.at(i).data(), nullptr, isSelected))
                {
                    // Special case
                    if (enumValue == 0)
                    {
                        // Full clear value
                        value = 0;
                    }
                    else
                    {
                        if (isSelected)
                        {
                            // Value was previously set, remove it
                            value &= ~enumValue;
                        }
                        else
                        {
                            // Value was previously not set, add it
                            value |= enumValue;
                        }
                    }

                    // Update value
                    *metadata.obj = static_cast<MemberT>(value);
                }
            }
        
            ImGui::EndCombo();
        }

        return static_cast<size_t>(oldValue) != static_cast<size_t>(*metadata.obj);
    }
    else if constexpr (Reflection::HasAttribute<Reflection::EnumRadioButton, DescriptorT>())
    {
        ImGui::SeparatorText(metadata.name);

        constexpr size_t size = enumNames.size();

        bool_t changed = false;
        for (size_t i = 0; i < size; i++)
        {
            // Get enum value
            const int32_t enumValue = static_cast<int32_t>(magic_enum::enum_value<MemberT>(i));

            changed |= ImGui::RadioButton(enumNames.at(i).data(), reinterpret_cast<int32_t*>(metadata.obj), enumValue);
        }

        return changed;
    }
    else
    {
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
}

#pragma endregion
