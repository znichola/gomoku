#pragma once
#include <random>

#include "Server.hpp"
#include "GameState.hpp"
#include "AI.hpp"

extern uint64_t zob[361][3];

void init_zobrist(uint64_t seed);

Cell parseCell(const std::string& s);
std::vector<Cell> parseGridCSV(const std::string& s);

bool handleLoadAIState(const Server::QueryMap& query, GameState& gs);

// DEBUG FUNCTION FOR WATCHER
void handleLoadGameState(const Server::QueryMap& query, GameState& gs);
