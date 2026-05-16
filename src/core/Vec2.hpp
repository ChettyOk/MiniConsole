#pragma once

#include <cmath>

namespace mc {

struct Vec2 {
    float x = 0.f;
    float y = 0.f;

    Vec2() = default;
    Vec2(float px, float py) : x(px), y(py) {}

    Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }

    Vec2& operator+=(Vec2 o) {
        x += o.x;
        y += o.y;
        return *this;
    }

    float lengthSq() const { return x * x + y * y; }
    float length() const { return std::sqrt(lengthSq()); }

    Vec2 normalized() const {
        const float len = length();
        if (len < 1e-6f) {
            return {0.f, 0.f};
        }
        return {x / len, y / len};
    }
};

inline float dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

} // namespace mc
