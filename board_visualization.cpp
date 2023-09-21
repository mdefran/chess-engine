#include <iostream>
#include "bitboard.h"
#include "chessboard.h"

void printBitboard(Bitboard bitboard) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 7; file >= 0; file--) {
            int square = rank * 8 + file;
            Bitboard mask = 1ULL << square;
            if (bitboard & mask)
                std::cout << "1 ";
            else
                std::cout << "0 ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void printChessboard(Chessboard &chessboard) {
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            Bitboard mask = BITBOARD(square);
            // Find the piece on the square and print it
            if (chessboard.whitePawns & mask)
                std::cout << "P ";
            else if (chessboard.whiteKnights & mask)
                std::cout << "N ";
            else if (chessboard.whiteBishops & mask)
                std::cout << "B ";
            else if (chessboard.whiteRooks & mask)
                std::cout << "R ";
            else if (chessboard.whiteQueen & mask)
                std::cout << "Q ";
            else if (chessboard.whiteKing & mask)
                std::cout << "K ";
            else if (chessboard.blackPawns & mask)
                std::cout << "p ";
            else if (chessboard.blackKnights & mask)
                std::cout << "n ";
            else if (chessboard.blackBishops & mask)
                std::cout << "b ";
            else if (chessboard.blackRooks & mask)
                std::cout << "r ";
            else if (chessboard.blackQueen & mask)
                std::cout << "q ";
            else if (chessboard.blackKing & mask)
                std::cout << "k ";
            else
                std::cout << ". ";
        }
        std::cout << std::endl;
    }
    std::cout << "  a b c d e f g h" << std::endl;
    std::cout << std::endl;
}