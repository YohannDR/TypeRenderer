#pragma once

#include "reflection.hpp"

class ContainersExample
{
public:
    std::vector<int32_t> vector;
    std::array<float_t, 5> array;
    uint16_t cArray[10];
    std::unordered_map<std::string, size_t> unorderedMap;

    void Print() const
    {
        for (auto& it : unorderedMap)
        {
            std::cout << it.first << " ; " << it.second << '\n';
        }

        std::cout << std::endl;
    }
};

REFL_AUTO(
    type(ContainersExample),

    field(vector),
    field(array),
    field(cArray),
    field(unorderedMap),

    func(Print)
);
