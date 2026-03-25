#include <iostream>
#include <vector>
#include <optional>

#include "Board.hpp"
#include <iomanip>

unsigned num_passed_tests = 0;
unsigned test_count = 0;

void isValidMove();

int main() {
    std::cout << "Testing Interpreter\n";
    std::cout << "===================\n";

    isValidMove();

    std::cout << "\nTests : " << num_passed_tests << "/" << test_count << "\n"; 
    return 0;
}


void isValidMove() {
    std::cout << "Testing isValidMove\n";

    using C = Board::Cell;

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

    for (const auto& [expected, moveToPlay, board, description] : testCases) {
        bool result = board.isValidMove(moveToPlay);
        if (result == expected) {
            std::cout << std::setw(2) << test_count << " [OK]\n";
            num_passed_tests++;
        } else {
            std::cout << std::setw(2) << test_count << " [KO] " << (description ? *description : "No description") << "\n";
        }
        test_count++;
    }
}