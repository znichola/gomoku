#pragma once
#include <ostream>
#include <stdint.h>

enum class Cell: uint8_t {
  EMPTY=0U,
  BLACK=1U,
  WHITE=2U,
  OUTSIDE=3U
};

std::ostream& operator<<(std::ostream& os, Cell c);
