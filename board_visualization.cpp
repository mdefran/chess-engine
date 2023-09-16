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
}

void printChessboard(Chessboard &chessboard) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 7; file >= 0; file--) {
            int square = rank * 8 + file;
            Bitboard mask = BITBOARD(square);
            // Find the piece on the square and print it
            if (chessboard.whitePawns & mask)
                std::cout << "♙ ";
            else if (chessboard.whiteKnights & mask)
                std::cout << "♘ ";
            else if (chessboard.whiteBishops & mask)
                std::cout << "♗ ";
            else if (chessboard.whiteRooks & mask)
                std::cout << "♖ ";
            else if (chessboard.whiteQueen & mask)
                std::cout << "♕ ";
            else if (chessboard.whiteKing & mask)
                std::cout << "♔ ";
            else if (chessboard.blackPawns & mask)
                std::cout << "♟︎ ";
            else if (chessboard.blackKnights & mask)
                std::cout << "♞ ";
            else if (chessboard.blackBishops & mask)
                std::cout << "♝ ";
            else if (chessboard.blackRooks & mask)
                std::cout << "♜ ";
            else if (chessboard.blackQueen & mask)
                std::cout << "♛ ";
            else if (chessboard.blackKing & mask)
                std::cout << "♚ ";
            else
                std::cout << ". ";
        }
        std::cout << std::endl;
    }
}