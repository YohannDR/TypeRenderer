#pragma once

#include "reflection.hpp"

class ContainersExample
{
public:
    std::vector<int32_t> vector;
    std::array<float_t, 5> array;
    uint16_t cArray[10];
};

REFL_AUTO(
    type(ContainersExample),

    field(vector),
    field(array),
    field(cArray)
);
