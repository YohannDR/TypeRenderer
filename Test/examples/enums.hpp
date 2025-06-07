#pragma once

#include "reflection.hpp"

enum class Enum
{
    Zero = 0,
    One = 1,
    Two = 2,
    Four = 4,
    Eight = 8,
    Ten = Two | Eight
};

constexpr Enum operator|(const Enum left, const Enum right)
{
    return static_cast<Enum>(static_cast<size_t>(left) | static_cast<size_t>(right));
}

/// @brief Example class for simple, basic types with no added behavior
class EnumExample
{
public:
    Enum enumNormal = Enum::One;
    Enum enumFlags = Enum::One | Enum::Two;
    Enum enumRadio = Enum::Eight;
};

REFL_AUTO(
    type(EnumExample),

    field(enumNormal),
    field(enumFlags, Reflection::EnumFlags()),
    field(enumRadio, Reflection::EnumRadioButton())
);
