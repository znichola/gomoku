#include <iostream>
#include <vector>
#include <optional>

#include "Board.hpp"
#include <iomanip>

unsigned num_passed_tests = 0;
unsigned test_count = 0;

void isValidMove();
void threeFreesPlayedPieceIsPartOf();

int main() {
    std::cout << "Testing Interpreter\n";
    std::cout << "===================\n";

    isValidMove();
    threeFreesPlayedPieceIsPartOf();

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
        {false, 16, Board{{C::EMPTY, C::EMPTY, C::EMPTY, C::EMPTY}}, "Out of bounds move"},
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

void threeFreesPlayedPieceIsPartOf() {
    std::cout << "\nTesting threeFreesPlayedPieceIsPartOf\n";
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
        {0, 1, Grid(7), "Move that's intended move is blank, it can't be part of a freeThree, invalid input"},
        {1, 2, Grid(7).setWhite(2).setWhite(3).setWhite(4), "White plays in the middle of three adjacent white pieces, it's part of a freeThree"},
        {0, 2, Grid(7).setWhite(2).setWhite(3).setWhite(4).setBlack(5), "White played on id 2, a three is formed but black is blocking on id 5"},
        {0, 1, Grid(7).setWhite(0).setWhite(1).setWhite(2), "White played on id 1, a three is formed but the board edge is blocking before id 0"},
        {1, 3, Grid(7).setWhite(2).setWhite(3).setWhite(5), "White plays a disconnected three, 1 0 1 1, and is sufficiently far from the edges"},
        {1, 1, Grid(7).setWhite(1).setWhite(2).setWhite(3), "White plays a connected three, and it's sufficiently far from the edges"},
    };

    for (const auto& [expected, moveToPlay, grid, description] : testCases) {
        unsigned result = grid.threeFreesPlayedPieceIsPartOf(moveToPlay);
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