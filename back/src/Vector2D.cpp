#include <iostream>
#include <Vector2D.hpp>

Vector2D::Vector2D(long x, long y) : x(x), y(y) {}

Vector2D::Vector2D(const Vector2D& other) {
    *this = other;
}

Vector2D& Vector2D::operator=(const Vector2D& other) {
    this->x = other.x;
    this->y = other.y;
    return (*this);
}

Vector2D::~Vector2D() {

}

void Vector2D::initCache(unsigned width) {
    cache.initialize(width);
}

Vector2D Vector2D::createFromIndex(long id, unsigned width) {
    (void) width;
    // return Vector2D(id % width, id / width);
    return cache.get(id);
}

long Vector2D::toIndex(unsigned width) const {
    return y * width + x;
}

Vector2D Vector2D::operator+(const Vector2D& other) const {
    return {x + other.x, y + other.y};
}

Vector2D Vector2D::operator-(const Vector2D& other) const {
    return {x - other.x, y - other.y};
}

Vector2D Vector2D::operator*(long k) const {
    return {x * k, y * k};
}

bool Vector2D::operator>(long value) const {
    return x > value && y > value;
}

bool Vector2D::operator<(long value) const {
    return x < value && y < value;
}

bool Vector2D::operator>=(long value) const {
    return x >= value && y >= value;
}

bool Vector2D::operator<=(long value) const {
    return x <= value && y <= value;
}

long Vector2D::dot(const Vector2D& other) const {
    return x * other.x + y * other.y;
}

long Vector2D::cross(const Vector2D& other) const {
    return x * other.y - y * other.x;
}

std::ostream& operator<<(std::ostream& os, const Vector2D& v) {
    return os << "(" << v.x << ", " << v.y << ")";
}

Vector2DCache Vector2D::cache = Vector2DCache();
