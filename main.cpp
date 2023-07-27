#include <stdio.h>
#include <iostream>
#include "bitboard.h"
#include "move.h"
#include "board_visualization.h"
#include "types.h"

int main () {
    Chessboard chessboard;
    Move move = Move(static_cast<unsigned short>(Square::e2), static_cast<unsigned short>(Square::e4), Move::DoublePawnPush);
    printChessboard(chessboard);
    chessboard.push(move);
    printChessboard(chessboard);
}