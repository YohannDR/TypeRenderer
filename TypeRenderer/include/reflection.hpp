#pragma once

#include <refl-cpp/include/refl.hpp>

#include "macros.hpp"
#include "meta_programming.hpp"

/// @file reflection.hpp
/// @brief Defines reflection types and helpers

/// @brief refl::type_descriptor<T> shorthand
/// @tparam T Type
template <typename T>
using TypeDescriptor = refl::type_descriptor<T>;

/// @brief refl::attr::usage::type shorthand
///
/// Specifies that an attribute can only be used on a type
using TypeAttribute = refl::attr::usage::type;

/// @brief refl::attr::usage::field shorthand
///
/// Specifies that an attribute can only be used on a field
using FieldAttribute = refl::attr::usage::field;

/// @brief refl::attr::usage::function shorthand
///
/// Specifies that an attribute can only be used on a function
using FunctionAttribute = refl::attr::usage::function;

/// @brief refl::attr::usage::member shorthand
///
/// Specifies that an attribute can only be used on a member (field and function)
using MemberAttribute = refl::attr::usage::member;

/// @brief Macro used to implement reflection for a class
#define REFLECTABLE_IMPL(type)                        \
private:                                              \
friend struct refl_impl::metadata::type_info__<type>; \

/// @namespace Reflection
/// @brief Provides utility functions for reflection
namespace Reflection
{
    /// @brief Checks to @c T is reflected (has a REFL_AUTO definition)
    /// @tparam T Type
    template <typename T>
    constexpr bool_t IsReflected = refl::trait::is_reflectable_v<T>;

    /// @brief Checks if the member descriptor describes a function
    /// @tparam DescriptorT Member descriptor
    template <typename DescriptorT>
    constexpr bool_t IsFunction = refl::trait::is_function_v<DescriptorT>;
    
    /// @brief Allows a boolean to be set to true when the field is modified
    /// @tparam T Parent type
    template <typename T>
    struct NotifyChange : FieldAttribute
    {
        /// @brief Shorthand for a class member pointer
        using PtrType = bool_t T::*;

        /// @brief Pointer to the boolean
        PtrType pointer;

        /// @brief Creates a notify change attribute using a pointer to a boolean inside the concerned class
        /// @param ptr Boolean pointer in the class
        constexpr explicit NotifyChange(const PtrType ptr) : pointer(ptr) {}
    };

    /// @brief Allows a callback to be called when a element of object is modified
    /// @tparam T Modified type
    template <typename T>
    struct ModifiedCallback : FieldAttribute
    {
        using Type = void(*)(T*);

        Type callback;

        constexpr explicit ModifiedCallback(Type&& c) : callback(std::move(c)) {}
    };

    /// @brief Display a field as read only
    struct ReadOnly : FieldAttribute
    {
    };

    /// @brief Allows an enum to be treated as a list of binary flags
    struct EnumFlags : FieldAttribute
    {
    };

    /// @brief Allows an integer or floating type to be bound between a minimum and a maximum value, it will display the field using a slider
    /// @tparam T Field type
    template <typename T>
    struct Range : FieldAttribute
    {
        static_assert(Meta::IsIntegralNumericOrFloating<T>, "Range attribute can only be used on integral numeric types, or floating types");
        
        /// @brief Minimum value
        T minimum;
        /// @brief Maximum value
        T maximum;

        /// @brief Creates a range
        /// @param min Minimum
        /// @param max Maximum
        constexpr explicit Range(const T& min, const T& max) : minimum(min), maximum(max) {}
    };

    /// @brief Allows an integer or floating type to be bound between a dynamic minimum and a maximum value, it will display the field using a slider
    /// @tparam ReflectT Top level type
    /// @tparam T Field type
    template <typename ReflectT, typename T>
    struct DynamicRange : FieldAttribute
    {
        static_assert(Meta::IsIntegralNumericOrFloating<T>, "Dynamic range attribute can only be used on integral numeric types, or floating types");

        /// @brief Shorthand for a class member pointer
        using PtrType = T ReflectT::*;

        /// @brief Minimum value
        PtrType minimum;
        /// @brief Maximum value
        PtrType maximum;

        /// @brief Creates a dynamic range with 0 being the minimum
        /// @param max Maximum
        constexpr explicit DynamicRange(const PtrType max) : minimum(nullptr), maximum(max) {}

        /// @brief Creates a dynamic range
        /// @param min Minimum
        /// @param max Maximum
        constexpr explicit DynamicRange(const PtrType min, const PtrType max) : minimum(min), maximum(max) {}
    };

    /// @brief Allows a tooltip to be bound to a member
    struct Tooltip : MemberAttribute
    {
        /// @brief Tooltip text
        const char_t* text;

        /// @brief Creates a tooltip from a string literal
        /// @param t Tooltip text
        constexpr explicit Tooltip(const char_t* const t) : text(t) {}
    };

    /// @brief Allows a dynamic tooltip to be bound to a member
    /// @tparam ReflectT Top level type
    template <typename ReflectT>
    struct DynamicTooltip : MemberAttribute
    {
        /// @brief Shorthand for a class member pointer
        using PtrType = std::string ReflectT::*;
        
        /// @brief Tooltip text
        const PtrType text;

        /// @brief Creates a tooltip from a dynamic string
        /// @param t Tooltip text
        constexpr explicit DynamicTooltip(const PtrType t) : text(t) {}
    };

    /// @brief Gets the type info of a class
    /// @tparam ReflectT Type
    /// @return Type info
    template <typename ReflectT>
    constexpr TypeDescriptor<ReflectT> GetTypeInfo();

    /// @brief Checks if a descriptor has a specified attribute
    /// @tparam AttributeT Attribute type
    /// @tparam DescriptorT Descriptor type
    /// @return Result
    template <typename AttributeT, typename DescriptorT>
    constexpr bool_t HasAttribute();

    /// @brief Gets the specified attribute of a descriptor
    /// @tparam AttributeT Attribute type
    /// @tparam DescriptorT Descriptor type
    /// @return Attribute
    template <typename AttributeT, typename DescriptorT>
    constexpr const AttributeT& GetAttribute();

    /// @brief Tries to get a specified attribute of a descriptor
    /// @tparam AttributeT Attribute type
    /// @tparam DescriptorT Descriptor type
    /// @return Attribute, @c nullptr if not found
    template <typename AttributeT, typename DescriptorT>
    constexpr const AttributeT* TryGetAttribute();
}

template <typename ReflectT>
constexpr TypeDescriptor<ReflectT> Reflection::GetTypeInfo()
{
    return refl::reflect<ReflectT>();
}

template <typename AttributeT, typename DescriptorT>
constexpr bool_t Reflection::HasAttribute()
{
    return refl::descriptor::has_attribute<AttributeT, DescriptorT>(DescriptorT{});
}

template <typename AttributeT, typename DescriptorT>
constexpr const AttributeT& Reflection::GetAttribute()
{
    return refl::descriptor::get_attribute<AttributeT, DescriptorT>(DescriptorT{});
}

template <typename AttributeT, typename DescriptorT>
constexpr const AttributeT* Reflection::TryGetAttribute()
{
    if constexpr (HasAttribute<AttributeT, DescriptorT>())
        return &GetAttribute<AttributeT, DescriptorT>();
    else
        return nullptr;
}
