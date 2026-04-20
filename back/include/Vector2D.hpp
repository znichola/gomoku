#pragma once

#include <array>

struct Vector2D {
    long x, y;

    Vector2D(long x, long y);

    Vector2D(const Vector2D& other);
    constexpr Vector2D(int x_, int y_) : x(x_), y(y_) {}
    Vector2D& operator=(const Vector2D& other);
    ~Vector2D();

    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator-(const Vector2D& other) const;
    Vector2D operator*(long k) const;

    long dot(const Vector2D& other) const;
    long cross(const Vector2D& other) const;
};

inline const char* spinner[4] = {"—", "\\", "|", "/"};
inline const std::array<Vector2D, 8> EXTREMITIES = {{
    { 1,  0}, // right
    { 1,  1}, // bottom-right
    { 0,  1}, // bottom
    {-1,  1}, // bottom-left
    {-1,  0}, // left
    {-1, -1}, // top-left
    { 0, -1}, // top
    { 1, -1}  // top-right
}};
inline const Vector2D *extptr = EXTREMITIES.begin();
