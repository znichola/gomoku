#pragma once
#include <ostream>
#include <stdint.h>

enum class Cell: uint8_t {EMPTY=0U, BLACK=1U, WHITE=2U};

// std::ostream& operator<<(std::ostream& os, Cell c) {
//     switch (c) {
//         case Cell::EMPTY: os << "empty"; break;
//         case Cell::BLACK: os << "black"; break;
//         case Cell::WHITE: os << "white"; break;
//     }
//     return os;
// }