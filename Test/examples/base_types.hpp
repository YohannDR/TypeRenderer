#pragma once

#include "reflection.hpp"

/// @brief Example class for simple, basic types with no added behavior
class BaseTypesExample
{
public:
    uint8_t myUint8 = 0;
    int8_t myInt8 = 0;
    uint16_t myUint16 = 0;
    int16_t myInt16 = 0;
    uint32_t myUint32 = 0;
    int32_t myInt32 = 0;
    uint64_t myUint64 = 0;
    int64_t myInt64 = 0;
    float_t myFloat = 0.f;
    double_t myDouble = 0.f;
    bool_t myBool = false;
    std::string myString = "Hello world";
};

REFL_AUTO(
    type(BaseTypesExample),

    field(myUint8),
    field(myInt8),
    field(myUint16),
    field(myInt16),
    field(myUint32),
    field(myInt32),
    field(myFloat),
    field(myDouble),
    field(myBool),
    field(myString)
);
