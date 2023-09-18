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

    /*
    for (int i = 0; i < 5; i++) {
        pseudoLegalMoves = chessboard.generatePseudoLegalMoves();
        int rand = std::rand() % pseudoLegalMoves.size();
        pseudoLegalMoves[rand].printMove();
        printChessboard(chessboard);
        printf("\n");
        chessboard.push(pseudoLegalMoves[rand]);
    }
    */

    printChessboard(chessboard);
    Move move = Move(Square::e2, Square::e4, Move::DoublePawnPush);
    move.printMove();
    chessboard.push(move);
    printChessboard(chessboard);

    return 0;
}