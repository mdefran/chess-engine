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
    int moves = 0;

    printChessboard(chessboard);
    for (int i = 0; i < 100; i++) {
        pseudoLegalMoves = chessboard.generatePseudoLegalMoves();
        if (pseudoLegalMoves.empty()) {
            moves = i;
            break;
        }

        Move bestMove;

        int rand = std::rand() % pseudoLegalMoves.size();
        bestMove = pseudoLegalMoves[rand];

        for (int i = 0; i < pseudoLegalMoves.size(); i++) {
            Move move = pseudoLegalMoves[i];
            if (move.isCapture())
                bestMove = move;
        }

        chessboard.push(bestMove);
        printf("%d: ", i+1);
        bestMove.printMove();
        printChessboard(chessboard);
    }

    printChessboard(chessboard);
    for (int i = 0; i < moves; i++) {
        std::cout << "Undo: ";
        chessboard.pastMoves.back().printMove();
        chessboard.pop();
        printChessboard(chessboard);
    }

    return 0;
}