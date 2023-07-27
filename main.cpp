#include <stdio.h>
#include <iostream>
#include "bitboard.h"
#include "move.h"
#include "board_visualization.h"
#include "types.h"

int main () {
    Chessboard chessboard;
    Move move = Move(Square::b1, Square::c3, Move::Quiet);
    printChessboard(chessboard);
    chessboard.push(move);
    printChessboard(chessboard);
    chessboard.pop();
    printChessboard(chessboard);
}