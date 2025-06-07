#pragma once

#include <cstdint>

using char_t = char;
using bool_t = bool;

#define DEFAULT_COPY_MOVE_OPERATIONS(type)                  \
    type(const type& other) noexcept = default;             \
    type(type&& other) noexcept = default;                  \
    type& operator=(const type& other) noexcept = default;  \
    type& operator=(type&& other) noexcept = default;

#define DELETE_COPY_MOVE_OPERATIONS(type)          \
    type(const type& other) = delete;              \
    type(type&& other) = delete;                   \
    type& operator=(const type& other) = delete;   \
    type& operator=(type&& other) = delete;

#define STATIC_CLASS(type)              \
    public:                             \
    type() = delete;                    \
    ~type() = delete;                   \
    DELETE_COPY_MOVE_OPERATIONS(type)   \
    private:

#define MAYBE_UNUSED [[maybe_unused]]
