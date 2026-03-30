#include <iostream>
#include <vector>
#include <optional>

#include "Board.hpp"
#include <iomanip>

unsigned num_passed_tests = 0;
unsigned test_count = 0;

void isValidMove();
void isDoubleThree();

int main() {
    std::cout << "Testing Interpreter\n";
    std::cout << "===================\n";

    isValidMove();
    isDoubleThree();

    std::cout << "\nTotals : " << num_passed_tests << "/" << test_count << "\n"; 
    return 0;
}


void isValidMove() {
    std::cout << "\nTesting isValidMove\n";
    unsigned before_test_count = test_count;
    unsigned before_passed_tests = num_passed_tests;

    using C = Cell;

    struct TestCase {
        bool expected;
        unsigned moveToPlay;
        Board board;
        std::optional<std::string> description;
    };

    std::vector<TestCase> testCases = {
        {false, 16, Board{Grid(4)}, "Out of bounds move"},
        {true, 1, Board{{C::BLACK, C::EMPTY, C::EMPTY, C::EMPTY}}, "Valid move"},
        {false, 0, Board{{C::WHITE, C::EMPTY, C::EMPTY, C::EMPTY}}, "Move onto occupied cell"},
    };

    for (auto& [expected, moveToPlay, board, description] : testCases) {
        bool result = board.isValidMove(moveToPlay);
        if (result == expected) {
            std::cout << std::setw(2) << test_count - before_test_count << " [OK]\n";
            num_passed_tests++;
        } else {
            std::cout << std::setw(2) << test_count - before_test_count << " [KO] " << (description ? *description : "No description") << "\n";
        }
        test_count++;
    }

    std::cout << "Result: " << num_passed_tests - before_passed_tests<< "/" << test_count - before_test_count << "\n";
}

void isDoubleThree() {
    std::cout << "\nTesting isDoubleThree\n";
    unsigned before_test_count = test_count;
    unsigned before_passed_tests = num_passed_tests;

    struct TestCase {
        unsigned expected;
        unsigned moveToPlay;
        Grid grid;
        std::optional<std::string> description;
    };

    /*  
        7x7
        0  1  2  3  4  5  6
        7  8  9 10 11 12 13
        14 15 16 17 18 19 20
        21 22 23 24 25 26 27
        28 29 30 31 32 33 34
        35 36 37 38 39 40 41

        By default it's white to play

        The initial Grid must include the indented move already played.
    */

    std::vector<TestCase> testCases = {
        // Horizontal free three checks
        //  Blocking checks
        {0, 1, Grid(7), "Move that's played is blank, it can't be part of a freeThree, invalid input"}
        ,{0, 2, Grid(7).setWhite({2, 3}), "White playes but it's only forming a set of two"}
        ,{0, 2, Grid(7).setWhite({1, 2, 5, 6}), "White playes but it's only forming a set of two, with distant white pieces"}
        ,{0, 4, Grid(7).setWhite(4), "White playes a single piece, no threes can be formed"}
        ,{0, 4, Grid(7).setWhite({2, 3, 4, 5}), "White plays, but a set of 4 is formed, should not count as a freeThree"} // TODO : is this true?
        ,{0, 3, Grid(7).setWhite({2, 3, 4, 5}), "White plays a set of disconnnected 4 is formed, should not count as freeThree"} // TODO : is this true?
        ,{0, 2, Grid(7).setWhite({2, 3, 4, 5}), "White played on id 2, a three is formed but black is blocking on id 5"}
        ,{0, 1, Grid(7).setWhite({0, 1, 2}), "White played on id 1, a three is formed but the board edge is blocking before id 0"}
        ,{0, 3, Grid(7).setWhite({1, 3, 5}), "White plays but pieces are all too spaced out"}
        ,{0, 1, Grid(7).setWhite({1, 2, 3, 4}), "White plays to form a disconnected 3, but black is blocking in between"}
        ,{0, 3, Grid(7).setWhite({2, 3, 4}).setBlack({0, 6}), "White makes a connected 3 but is flanked by black one tile out. There is way to have 4 without one touching an edge, so it's not a free three"} // TODO : check
        ,{0, 4, Grid(7).setWhite({2, 4, 5}).setBlack(1), "White makes disconneced 3, but black flanks and so block a freeThree"}
        // Three free formed
        ,{1, 2, Grid(7).setWhite({2, 3, 4}), "White plays in the middle of adjacent white pieces, it's part of a freeThree"}
        ,{1, 3, Grid(7).setWhite({1, 2, 3}).setBlack(5), "White plays a three, back piece is too far to block"} // TODO : is this true?
        ,{1, 3, Grid(7).setWhite({2, 3, 5}), "White plays a disconnected three, 1 0 1 1, and is sufficiently far from the edges"}
    };

    for (const auto& [expected, moveToPlay, grid, description] : testCases) {
        unsigned result = grid.isDoubleThree(moveToPlay);
        if (result == expected) {
            std::cout << std::setw(2) << test_count - before_test_count << " [OK]\n";
            num_passed_tests++;
        } else {
            std::cout << std::setw(2) << test_count - before_test_count << " [KO] " 
                      << "Exp:" << expected << " Res:" << result << " - "
                      << (description ? *description : "No description") << "\n";
        }
        test_count++;
    }

    std::cout << "Result: " << num_passed_tests - before_passed_tests<< "/" << test_count - before_test_count << "\n";
}