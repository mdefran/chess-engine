#include <stdio.h>
#include <iostream>
#include "bitboard.h"
#include "move.h"

enum PieceType {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

enum Color {
    White,
    Black
};

/*
Assignment follows a right-to-left, bottom-to-top pattern in reference to the corresponding positions on a chessboard.
Since H1 = 0, the LSB corresponds to the bottom right of the board.
This varies from traditional little-endian encoding of both the ranks and files of the board to provide visual clarity.
*/
enum Square {
    h1, g1, f1, e1, d1, c1, b1, a1,
    h2, g2, f2, e2, d2, c2, b2, a2,
    h3, g3, f3, e3, d3, c3, b3, a3,
    h4, g4, f4, e4, d4, c4, b4, a4,
    h5, g5, f5, e5, d5, c5, b5, a5,
    h6, g6, f6, e6, d6, c6, b6, a6,
    h7, g7, f7, e7, d7, c7, b7, a7,
    h8, g8, f8, e8, d8, c8, b8, a8
};

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

struct Chessboard {
    // Member variables
    Bitboard whitePawns;
    Bitboard whiteKnights;
    Bitboard whiteBishops;
    Bitboard whiteRooks;
    Bitboard whiteQueen;
    Bitboard whiteKing;
    Bitboard whitePieces;

    Bitboard blackPawns;
    Bitboard blackKnights;
    Bitboard blackBishops;
    Bitboard blackRooks;
    Bitboard blackQueen;
    Bitboard blackKing;
    Bitboard blackPieces;

    Bitboard allPieces;

    Color turn;
    
    // Declare lookup tables for leaping pieces
    Bitboard knightAttacks[64], kingAttacks[64], whitePawnAttacks[64], blackPawnAttacks[64];

    // Move lists for move generation
    MoveList pseudoLegalMoves, legalMoves;

    // Variable to store double pawn moves that could enable an en passant
    unsigned short enPassant;
    bool whiteQueenCastle, whiteKingCastle, blackQueenCastle, blackKingCastle;

    Chessboard() {
        // Initialize the bitboards to match the piece's positions at the start of the game
        whitePawns = 0xFF00ULL;
        whiteKnights = 0x42ULL;
        whiteBishops = 0x24ULL;
        whiteRooks = 0x81ULL;
        whiteQueen = 0x10ULL;
        whiteKing = 0x08ULL;
        whitePieces = 0xFFFFULL;

        blackPawns = 0xFF000000000000ULL;
        blackKnights = 0x4200000000000000ULL;
        blackBishops = 0x2400000000000000ULL;
        blackRooks = 0x8100000000000000ULL;
        blackQueen = 0x1000000000000000ULL;
        blackKing = 0x0800000000000000ULL;
        blackPieces = 0xFFFF000000000000ULL;

        allPieces = whitePieces | blackPieces;

        turn = White;
        
        // Generate attack map lookup tables for knights and kings
        for (int square = 0; square < 64; square++) {
            Bitboard fromSquare = BITBOARD(square);

            kingAttacks[square] = north(fromSquare) | northeast(fromSquare) | east(fromSquare) | 
                                  southeast(fromSquare) | south(fromSquare) | southwest(fromSquare) |
                                  west(fromSquare) | northwest(fromSquare);

            knightAttacks[square] = (((fromSquare & ~(FILE_G | FILE_H | RANK_8)) << 6) | ((fromSquare & ~(FILE_G | FILE_H | RANK_1)) >> 10)) |
                                    (((fromSquare & ~(FILE_H | RANK_7 | RANK_8)) << 15) | ((fromSquare & ~(FILE_H | RANK_1 | RANK_2)) >> 17)) |
                                    (((fromSquare & ~(FILE_A | RANK_7 | RANK_8)) << 17) | ((fromSquare & ~(FILE_A | RANK_1 | RANK_2)) >> 15)) |
                                    (((fromSquare & ~(FILE_A | FILE_B | RANK_8)) << 10) | ((fromSquare & ~(FILE_A | FILE_B | RANK_1)) >> 6));

            whitePawnAttacks[square] = north(fromSquare);
            blackPawnAttacks[square] = south(fromSquare);
        }

        // Negative value used to indicate no possible en passant moves as it does not correspond to any square index
        enPassant = -1;

        // Players start out with all castling rights
        whiteQueenCastle = whiteKingCastle = blackQueenCastle = blackKingCastle = true;
    }

    // Determine if the opponent has a piece on a given square and return true if so
    bool enemyAt(unsigned short square) {
        Bitboard enemyPieces = (turn == White) ? blackPieces : whitePieces;
        return enemyPieces & BITBOARD(square);
    }

    /*
    // Efficiently store a move in the pseudo legal moves list
    void addMove(unsigned short fromSquare, unsigned short toSquare) {
        Move move = fromSquare | (toSquare << 6);
        pseudoLegalMoves.push_back(move);
    }
    */

    // Generate pseudo legal king moves for the current player using the attack maps
    void generateKingMoves() {
        unsigned short fromSquareIndex = (turn == White) ? POP_LSB(whiteKing) : POP_LSB(blackKing);
        Bitboard fromSquare = BITBOARD(fromSquareIndex);
        Bitboard toSquares = kingAttacks[fromSquareIndex];

        // Check for pseudo legal castling
        if (turn == White) {
            if (whiteKingCastle == true && (allPieces & (0x6ULL)) == 0) {
                // Implement white king side castle
            }
        }

        while (toSquares != 0) {
            addMove(fromSquareIndex, POP_LSB(toSquares));
        }
    }

    // Generate pseudo legal knight moves for the current player using the attack maps
    void generateKnightMoves() {
        Bitboard fromSquares = (turn == White) ? whiteKnights : blackKnights;
        while (fromSquares != 0) {
            unsigned short fromSquare = POP_LSB(fromSquares);
            Bitboard toSquares = knightAttacks[fromSquare];

            while (toSquares != 0) {
                addMove(fromSquare, POP_LSB(toSquares));
            }
        }
    }

    void generatePawnMoves() {
        Bitboard fromSquares = (turn == White) ? whitePawns : blackPawns;
        while (fromSquares != 0) {
            unsigned short fromSquareIndex = POP_LSB(fromSquares);
            Bitboard fromSquare = BITBOARD(fromSquareIndex);
            Bitboard toSquares = (turn == White) ? whitePawnAttacks[fromSquareIndex] : blackPawnAttacks[fromSquareIndex];

            // Consider conditional moves
            if (turn == White) {
                // Check for initial double advance conditions
                if ((fromSquare & RANK_2) != 0 && (((fromSquare << 8) | (fromSquare << 16)) & allPieces) == 0)
                    toSquares |= fromSquare << 16;
                // Check for diagonal pieces to capture
                if ((northeast(fromSquare) & blackPieces) != 0)
                    toSquares |= northeast(fromSquare);
                if ((northwest(fromSquare) & blackPieces) != 0)
                    toSquares |= northwest(fromSquare);
            } else {
                // Check for initial double advance conditions
                if ((fromSquare & RANK_7) != 0 && (((fromSquare >> 8) | (fromSquare >> 16)) & allPieces) == 0)
                    toSquares |= fromSquare >> 16;
                // Check for diagonal pieces to capture
                if ((southeast(fromSquare) & whitePieces) != 0)
                    toSquares |= southeast(fromSquare);
                if ((southwest(fromSquare) & whitePieces) != 0)
                    toSquares |= southwest(fromSquare);
            }

            // Add the possible moves
            while (toSquares != 0) {
                addMove(fromSquareIndex, POP_LSB(toSquares));
            }
        }
    }

    void printPseudoMoves() {
        for (const auto &move : pseudoLegalMoves) {
            unsigned short from = move & 0x3F;
            unsigned short to = (move >> 6) & 0x3F;
            std::cout << static_cast<Square>(from) << " " << static_cast<Square>(to) << std::endl;
        }
    }

    void printChessboard() {
        for (int rank = 7; rank >= 0; rank--) {
            for (int file = 7; file >= 0; file--) {
                int square = rank * 8 + file;
                Bitboard mask = BITBOARD(square);
                if (allPieces & mask) {
                    // Find the piece on the square and print it
                    if (whitePawns & mask)
                        std::cout << "♙ ";
                    else if (whiteKnights & mask)
                        std::cout << "♘ ";
                    else if (whiteBishops & mask)
                        std::cout << "♗ ";
                    else if (whiteRooks & mask)
                        std::cout << "♖ ";
                    else if (whiteQueen & mask)
                        std::cout << "♕ ";
                    else if (whiteKing & mask)
                        std::cout << "♔ ";
                    else if (blackPawns & mask)
                        std::cout << "♟︎ ";
                    else if (blackKnights & mask)
                        std::cout << "♞ ";
                    else if (blackBishops & mask)
                        std::cout << "♝ ";
                    else if (blackRooks & mask)
                        std::cout << "♜ ";
                    else if (blackQueen & mask)
                        std::cout << "♛ ";
                    else if (blackKing & mask)
                        std::cout << "♚ ";
                } else {
                    std::cout << ". ";
                }
            }
            std::cout << std::endl;
        }
    }

    void generateLegalMoves() {

    }
};

int main () {
    Chessboard chessboard;
    chessboard.generateKnightMoves();
    chessboard.printPseudoMoves();
    return 0;
}