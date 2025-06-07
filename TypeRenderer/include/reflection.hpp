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

    template <typename>
    struct StructGetMemberT
    {
        using Type = decltype(nullptr);
    };
    
    template <typename T, size_t N>
    struct StructGetMemberT<refl::function_descriptor<T, N>>
    {
        using Type = decltype(refl::function_descriptor<T, N>::pointer);
    };

    template <typename T, size_t N>
    struct StructGetMemberT<refl::field_descriptor<T, N>>
    {
        using Type = Meta::RemoveConstSpecifier<typename refl::field_descriptor<T, N>::value_type>;
    };

    /// @brief Gets the member type declaration, whether it's a field or a function
    /// @tparam DescriptorT Member descriptor
    template <typename DescriptorT>
    using GetMemberT = typename StructGetMemberT<DescriptorT>::Type;

    /// @brief Gets the type info of a class
    /// @tparam ReflectT Type
    /// @return Type info
    template <typename ReflectT>
    constexpr TypeDescriptor<ReflectT> GetTypeInfo() { return refl::reflect<ReflectT>(); }

    /// @brief Checks if a descriptor has a specified attribute
    /// @tparam AttributeT Attribute type
    /// @tparam DescriptorT Descriptor type
    /// @return Result
    template <typename AttributeT, typename DescriptorT>
    constexpr bool_t HasAttribute() { return refl::descriptor::has_attribute<AttributeT, DescriptorT>(DescriptorT{}); }

    /// @brief Gets the specified attribute of a descriptor
    /// @tparam AttributeT Attribute type
    /// @tparam DescriptorT Descriptor type
    /// @return Attribute
    template <typename AttributeT, typename DescriptorT>
    constexpr const AttributeT& GetAttribute() { return refl::descriptor::get_attribute<AttributeT, DescriptorT>(DescriptorT{}); }
}

namespace Reflection
{
    /// @brief Display a field as read only
    struct ReadOnly : FieldAttribute
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


    /// @brief Allows a boolean to be set to true when the field is modified
    /// @tparam ReflectT Parent type
    template <typename ReflectT>
    struct NotifyChange : FieldAttribute
    {
        /// @brief Shorthand for a class member pointer
        using PtrType = bool_t ReflectT::*;

        /// @brief Pointer to the boolean
        PtrType pointer;

        /// @brief Creates a notify change attribute using a pointer to a boolean inside the concerned class
        /// @param ptr Boolean pointer in the class
        constexpr explicit NotifyChange(const PtrType ptr) : pointer(ptr) {}
    };

    /// @brief Allows a callback to be called when a element of object is modified
    /// @tparam ReflectT Parent type
    template <typename ReflectT>
    struct ModifiedCallback : FieldAttribute
    {
        using Type = void(*)(ReflectT*);

        Type callback;

        constexpr explicit ModifiedCallback(Type&& c) : callback(std::move(c)) {}
    };
    

    /// @brief Allows an enum to be treated as a list of binary flags
    struct EnumFlags : FieldAttribute
    {
    };

    /// @brief Allow an enum to be displayed as radio buttons
    struct EnumRadioButton : FieldAttribute
    {
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

    /// @brief Allows to change the display name of the member, overriding the default name
    struct CustomName : MemberAttribute
    {
        /// @brief Custom name text
        const char_t* text;

        /// @brief Creates a custom name from a string literal
        /// @param t Tooltip text
        constexpr explicit CustomName(const char_t* const t) : text(t) {}
    };
}
