#pragma once

#include "reflection.hpp"

class ContainersExample
{
public:
    std::vector<int32_t> vector;
    std::array<float_t, 5> array;
    uint16_t cArray[10];

    void AddedElement(int32_t& element)
    {
        std::cout << "Added element " << element << " to the vector" << std::endl;
    }
};

REFL_AUTO(
    type(ContainersExample),

    field(vector, Reflection::ContainerAddCallback(&ContainersExample::AddedElement)),
    field(array),
    field(cArray)
);
