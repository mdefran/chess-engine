#include "chessboard.h"

Chessboard::Chessboard() {
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

void Chessboard::push(Move move) {

}

void Chessboard::pop() {

}

Move Chessboard::peek() {

}

bool Chessboard::isCheck() {

}

bool Chessboard::isCheckmate() {

}

Color Chessboard::winningSide() {

}