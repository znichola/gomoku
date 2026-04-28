#pragma once
#include <ostream>

namespace AI {
  struct Eval {
      float black = 0;
      float white = 0;

      Eval operator-(const Eval& other) const;
      Eval operator+(const Eval& other) const;
      Eval operator*(const Eval& other) const;
      Eval operator/(const Eval& other) const;

      Eval& operator+=(const Eval& other);

      Eval operator-(float scale) const;
      Eval operator+(float scale) const;
      Eval operator*(float scale) const;
      Eval operator/(float scale) const;
  };

  Eval operator-(float scale, const Eval& other);
  Eval operator+(float scale, const Eval& other);
  Eval operator*(float scale, const Eval& other);
  Eval operator/(float scale, const Eval& other);

  inline std::ostream &operator<<(std::ostream &os, const Eval &eval) {
      os << "[" << eval.black << ", " << eval.white << "]";
      return os;
  }

  struct EvalGroups {
      Eval open;
      Eval half;
  };
};