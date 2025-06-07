#pragma once

#include <iostream>

#include "reflection.hpp"

struct TestStruct
{
    REFLECTABLE_IMPL(TestStruct)

    int32_t myInt = 50;
    double_t myDouble = 3.14f;
    std::string myString = "Hello world";

    void MyFunction() const
    {
        std::cout << myInt << " ; " << myDouble << " ; " << myString << '\n';
    }
};

REFL_AUTO(
    type(TestStruct),
    field(myInt),
    field(myDouble),
    field(myString, Reflection::DynamicTooltip(&TestStruct::myString)),

    func(MyFunction, Reflection::DynamicTooltip(&TestStruct::myString))
);
