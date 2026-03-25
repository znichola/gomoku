#include <iostream>
#include <vector>

#include "Board.hpp"

unsigned num_failed_tests;
unsigned num_passed_tests;

void isFreeMove();

int main() {
    std::cout << "Testing Interpreter\n";
    std::cout << "===================\n";

    isFreeMove();

    std::cout << "\nTests : " << num_passed_tests 
    << "/" << num_passed_tests + num_failed_tests << "\n"; 
    return 0;
}

void isFreeMove() {
    std::cout << "Testing isFreeMove\n";

    using enum Board::Cell;

    struct TestCase {
        bool expected;
        unsigned moveToPlay;
        Board board;
    };

    std::vector<TestCase> testCases = {
        {true, 16, Board{{EMPTY, EMPTY, EMPTY, EMPTY}, 4}},
        {false, 16, Board{{BLACK, EMPTY, EMPTY, EMPTY}, 4}},
        {false, 16, Board{{WHITE, EMPTY, EMPTY, EMPTY}, 4}},
    };

    Board board = Board{{EMPTY, EMPTY, EMPTY, EMPTY}, 4};
    bool res = board.isValidMove(16);
}