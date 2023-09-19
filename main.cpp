#include <stdio.h>
#include <iostream>
#include "bitboard.h"
#include "move.h"
#include "board_visualization.h"
#include "types.h"
#include "chessboard.h"

// For random moves
#include <cstdlib>
#include <ctime>
#include <random>
int main () {
    Chessboard chessboard;
    MoveList pseudoLegalMoves;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    printChessboard(chessboard);
    printf("\n");
    for (int i = 0; i < 15; i++) {
        pseudoLegalMoves = chessboard.generatePseudoLegalMoves();
        int rand = std::rand() % pseudoLegalMoves.size();
        chessboard.push(pseudoLegalMoves[rand]);
        pseudoLegalMoves[rand].printMove();
        printChessboard(chessboard);
        printf("\n");
    }

    return 0;
}