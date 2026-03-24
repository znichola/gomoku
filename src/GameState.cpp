#include "GameState.hpp"

std::string GameState::serialize() const {
    return board.serialize();
}