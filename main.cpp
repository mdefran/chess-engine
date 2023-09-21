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

int main1() {
    Chessboard chessboard;
    MoveList pseudoLegalMoves = chessboard.generatePseudoLegalMoves();

    printChessboard(chessboard);
    for (int i = 0; i < pseudoLegalMoves.size(); i++) {
        Move pseudoLegalMove = pseudoLegalMoves[i];
        pseudoLegalMove.printMove();
    }

    return 0;
}

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