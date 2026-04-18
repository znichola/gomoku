#include "Cell.hpp"

std::ostream& operator<<(std::ostream& os, Cell c) {
    switch (c) {
        case Cell::EMPTY: os << "empty"; break;
        case Cell::BLACK: os << "black"; break;
        case Cell::WHITE: os << "white"; break;
        case Cell::OUTSIDE: os << "outside"; break;
    }
    return os;
}
