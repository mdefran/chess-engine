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

int main() {
    Chessboard chessboard;
    MoveList pseudoLegalMoves;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    printChessboard(chessboard);
    for (int i = 0; i < 75; i++) {
        pseudoLegalMoves = chessboard.generatePseudoLegalMoves();
        Move bestMove;

        int rand = std::rand() % pseudoLegalMoves.size();
        bestMove = pseudoLegalMoves[rand];

        for (int i = 0; i < pseudoLegalMoves.size(); i++) {
            Move move = pseudoLegalMoves[i];
            if (move.isCapture())
                bestMove = move;
        }

        chessboard.push(bestMove);
        bestMove.printMove();
        printChessboard(chessboard);
    }

    printChessboard(chessboard);
    for (int i = 0; i < 75; i++) {
        std::cout << "Undo: ";
        chessboard.pastMoves.back().printMove();
        chessboard.pop();
        printChessboard(chessboard);
    }

    return 0;
}