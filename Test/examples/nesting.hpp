#pragma once

#include "reflection.hpp"

struct SomeData
{
    float_t data1 = 0.f;
    int32_t data2 = 0;
    bool_t data3 = false;
};

class NestingExample
{
public:
    SomeData dataSingle;
    std::array<SomeData, 5> dataArray;
};

REFL_AUTO(
    type(SomeData),

    field(data1),
    field(data2),
    field(data3)
)

REFL_AUTO(
    type(NestingExample),

    field(dataSingle),
    field(dataArray)
)
