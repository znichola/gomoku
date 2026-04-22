#pragma once
#include <random>

#include "Server.hpp"
#include "GameState.hpp"

extern uint64_t zob[361][3];

void init_zobrist(uint64_t seed);

Cell parseCell(const std::string& s);
std::vector<Cell> parseGridCSV(const std::string& s);

// DEBUG FUNCTION FOR WATCHER
void handleLoadGameState(const Server::QueryMap& query, GameState& gs);
