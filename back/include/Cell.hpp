#pragma once
#include <ostream>
#include <stdint.h>

/// Represents a cell state on a Gomoku board : 0) EMPTY / 1) BLACK / 2) WHITE / 3) OUTSIDE
enum class Cell: uint8_t {
  EMPTY=0U,    ///< Empty cell
  BLACK=1U,    ///< Black stone
  WHITE=2U,    ///< White stone
  OUTSIDE=3U   ///< Outside the board
};

std::ostream& operator<<(std::ostream& os, Cell c);
