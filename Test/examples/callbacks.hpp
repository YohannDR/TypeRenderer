#pragma once

#include "reflection.hpp"

/// @brief Example class for simple, basic types with no added behavior
class CallbacksExample
{
public:
    int32_t valueBool = 0;
    bool_t valueBoolChanged = false;

    int32_t valueFunction = 0;
};

REFL_AUTO(
    type(CallbacksExample),

    field(valueBool, Reflection::NotifyChange(&CallbacksExample::valueBoolChanged)),
    field(valueBoolChanged, Reflection::ReadOnly()),
    field(valueFunction, Reflection::ModifiedCallback<CallbacksExample>([](CallbacksExample* const obj) -> void
    {
        std::cout << "valueFunction was modified : " << obj->valueFunction << std::endl;
    }))
);
