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
    MoveList legalMoves;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    int moves = 0;

    printChessboard(chessboard);
    while (true) {
        if (chessboard.isCheckmate()) {
            printf("WINNER: %d\n", chessboard.winner);
            break;
        }

        legalMoves = chessboard.generateLegalMoves();
        Move bestMove;

        if (legalMoves.empty()) {
            printf("STALEMATE");
            break;
        }

        int rand = std::rand() % legalMoves.size();
        bestMove = legalMoves[rand];

        for (int i = 0; i < legalMoves.size(); i++) {
            Move move = legalMoves[i];
            if (move.isCapture())
                bestMove = move;
        }

        chessboard.push(bestMove);
        printf("%d: ", moves+1);
        bestMove.printMove();
        printChessboard(chessboard);
        moves++;
    }

    return 0;
}