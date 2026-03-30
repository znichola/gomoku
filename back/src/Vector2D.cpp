#include <iostream>

struct Vector2D {
    long x, y;

    // Constructeur
    Vector2D(long x = 0, long y = 0) : x(x), y(y) {}

    // Addition
    Vector2D operator+(const Vector2D& other) const {
        return {x + other.x, y + other.y};
    }

    // Soustraction
    Vector2D operator-(const Vector2D& other) const {
        return {x - other.x, y - other.y};
    }

    // Multiplication par un scalaire
    Vector2D operator*(long k) const {
        return {x * k, y * k};
    }

    // Produit scalaire
    long dot(const Vector2D& other) const {
        return x * other.x + y * other.y;
    }

    // Déterminant (utile pour colinéarité)
    long cross(const Vector2D& other) const {
        return x * other.y - y * other.x;
    }

    // Affichage
    friend std::ostream& operator<<(std::ostream& os, const Vector2D& v) {
        return os << "(" << v.x << ", " << v.y << ")";
    }
};
