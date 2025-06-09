#pragma once

#include "reflection.hpp"

struct ContainersStructExample
{
    float_t value1;
    size_t value2;
    bool_t value3;
    int16_t value4;
};

class ContainersExample
{
public:
    std::vector<int32_t> vector;
    std::array<float_t, 5> array;
    uint16_t cArray[10];
    std::vector<ContainersStructExample> arrayStruct;
    std::vector<std::pair<float_t, bool_t>> arrayPair;

    void AddedElement(int32_t& element)
    {
        std::cout << "Added element " << element << " to the vector" << std::endl;
    }
};

REFL_AUTO(
    type(ContainersStructExample),

    field(value1),
    field(value2),
    field(value3),
    field(value4)
)

REFL_AUTO(
    type(ContainersExample),

    field(vector, Reflection::ContainerAddCallback(&ContainersExample::AddedElement)),
    field(array, Reflection::PaddingY(20.f), Reflection::PaddingX(20.f)),
    field(cArray),
    field(arrayStruct),
    field(arrayPair)
);
