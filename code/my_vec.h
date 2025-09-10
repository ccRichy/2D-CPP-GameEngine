#pragma once
#include "my_types_constants.h"

struct Vec2
{
    float32 x;
    float32 y;

    Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this; // return the modified object
    }
    Vec2 operator+(const Vec2& other) const {
        Vec2 result = *this;
        result += other;
        return result;
    }
    Vec2& operator-=(const Vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this; // return the modified object
    }
    Vec2 operator-(const Vec2& other) const {
        Vec2 result = *this;
        result -= other;
        return result;
    }
};
