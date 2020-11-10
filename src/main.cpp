#include "main.h"
#include "data.h"
#include "perft.h"


int main()
{
    constexpr Bitboard startingBoardP0 = 0b11111000000000000000000000000011;
    constexpr Bitboard startingBoardP1 = 0b00000000000000000000111110001100;
    constexpr Bitboard startingBoardKings = 0b00100000000000000000001000010000;

    State startState = {{startingBoardP0, startingBoardP1}, startingBoardKings};
    printIncreasingPerftSpeed(startState, 8, 0);

#ifndef _DEBUG
    std::cout << "Finished. Press enter to exit." << std::endl;
    getchar();
#endif
    return 0;
}
