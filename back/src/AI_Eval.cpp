#include "AI_Eval.hpp"

AI::Eval AI::Eval::operator+(const Eval& other) const {
    return { black + other.black, white + other.white };
}

AI::Eval AI::Eval::operator-(const Eval& other) const {
    return { black - other.black, white - other.white };
}

AI::Eval AI::Eval::operator*(const Eval& other) const {
    return { black * other.black, white * other.white };
}

AI::Eval AI::Eval::operator/(const Eval& other) const {
    return { black / other.black, white / other.white };
}


AI::Eval& AI::Eval::operator+=(const Eval& other) {
    black += other.black;
    white += other.white;
    return *this;
}


AI::Eval AI::Eval::operator-(float scale) const {
    return { black - scale, white - scale };
}

AI::Eval AI::Eval::operator+(float scale) const {
    return { black + scale, white + scale };
}

AI::Eval AI::Eval::operator*(float scale) const {
    return { black * scale, white * scale };
}

AI::Eval AI::Eval::operator/(float scale) const {
    return { black / scale, white / scale };
}


AI::Eval AI::operator-(float scale, const Eval& other) {
    return { scale - other.black, scale - other.white };
}

AI::Eval AI::operator+(float scale, const Eval& other) {
    return { scale + other.black, scale + other.white };
}

AI::Eval AI::operator*(float scale, const Eval& other) {
    return { scale * other.black, scale * other.white };
}

AI::Eval AI::operator/(float scale, const Eval& other) {
    return { scale / other.black, scale / other.white };
}