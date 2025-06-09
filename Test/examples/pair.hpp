#pragma once

#include "reflection.hpp"

class PairExample
{
public:
    std::pair<float_t, size_t> pair1;
    std::pair<std::string, bool_t> pair2;
    std::pair<int16_t, double_t> pair3;
};

REFL_AUTO(
    type(PairExample),

    field(pair1, Reflection::PairName("This is a float", "This is a size_t")),
    field(pair2),
    field(pair3)
);
