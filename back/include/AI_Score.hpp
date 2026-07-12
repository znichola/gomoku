#pragma once

/*
    Two named aliases for the two score conventions used throughout AI.cpp, so
    signatures and locals say which one is meant instead of leaving it to a
    comment. Plain aliases, not distinct types - the compiler won't catch a
    mix-up, but every declaration now documents its convention.

    AbsScore - fixed polarity: positive favors white, negative favors black,
    independent of whose turn it is. evaluate()'s native currency.

    RelScore - negamax convention: positive favors whoever is to move at THIS
    node. Flips sign every ply. What the search functions and the TT operate in.

    color is the sign of whoever is to move at the node a score belongs to:
    +1.0f if white, -1.0f if black. That's the only place the two conventions
    are meant to touch - see evaluate()'s TT probe/store for why this matters
    (a RelScore read back as an AbsScore without going through this is wrong
    whenever black is to move at that node).
*/
namespace AI {
    using AbsScore = float;
    using RelScore = float;

    constexpr RelScore toRelative(AbsScore s, float color) { return color * s; }
    constexpr AbsScore toAbsolute(RelScore s, float color) { return color * s; }
}
