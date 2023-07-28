#include "chessboard.h"
#include "move.h"
#include "bitboard.h"
#include "types.h"

// Declare lookup tables for leaping pieces
Bitboard knightAttacks[64], kingAttacks[64], whitePawnAttacks[64], blackPawnAttacks[64];

// Initialize lookup tables
void initializeAttackTables() {
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
}

// Generate pseudo legal king moves for the current player using the attack maps and checking for castling rights
void generateKingMoves(Chessboard &chessboard) {
    unsigned short fromSquareIndex = (chessboard.turn == White) ? POP_LSB(chessboard.whiteKing) : POP_LSB(chessboard.blackKing);
    Bitboard fromSquare = BITBOARD(fromSquareIndex);
    Bitboard toSquares = kingAttacks[fromSquareIndex];

    // Check for pseudo legal castling
    Move move;
    if (chessboard.turn == White) {
        if (chessboard.whiteKingCastle == true && (chessboard.allPieces & (0x6ULL)) == 0) {
            move = Move(Square::e1, Square::g1, Move::KingCastle);
            chessboard.pseudoLegalMoves.push_back(move);
        }
        if (chessboard.whiteQueenCastle == true && (chessboard.allPieces & (0x70ULL)) == 0) {
            move = Move(Square::e1, Square::c1, Move::QueenCastle);
            chessboard.pseudoLegalMoves.push_back(move);
        }
    } else {
        if (chessboard.blackKingCastle == true && (chessboard.allPieces & (0x600000000000000ULL)) == 0) {
            move = Move(Square::e8, Square::g8, Move::KingCastle);
            chessboard.pseudoLegalMoves.push_back(move);
        }
        if (chessboard.blackQueenCastle == true && (chessboard.allPieces & (0x7000000000000000) == 0)) {
            move = Move(Square::e8, Square::c8, Move::QueenCastle);
            chessboard.pseudoLegalMoves.push_back(move);
        }
    }

    // Add normal moves
    while (toSquares != 0) {
        unsigned short toSquare = POP_LSB(toSquares);
        Move::MoveType type = (toSquare & ((chessboard.turn == White) ? chessboard.blackPieces : chessboard.whitePieces) != 0) ? Move::Capture : Move::Quiet;
        chessboard.pseudoLegalMoves.push_back(Move(fromSquareIndex, toSquare, type));
    }
}

// Generate pseudo legal knight moves for the current player using the attack maps
void generateKnightMoves(Chessboard &chessboard) {
    Bitboard fromSquares = (chessboard.turn == White) ? chessboard.whiteKnights : chessboard.blackKnights;
    while (fromSquares != 0) {
        unsigned short fromSquare = POP_LSB(fromSquares);
        Bitboard toSquares = knightAttacks[fromSquare];

        Move move;
        while (toSquares != 0) {
            unsigned short toSquare = POP_LSB(toSquares);
            Move::MoveType type = (toSquare & ((chessboard.turn == White) ? chessboard.blackPieces : chessboard.whitePieces) != 0) ? Move::Capture : Move::Quiet;
            chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, type));
        }
    }
}

// Generate pseudo legal pawn moves for the current player using the attack maps and manually checking for double advances, captures, and en passant
void generatePawnMoves(Chessboard &chessboard) {
    Bitboard fromSquares = (chessboard.turn == White) ? chessboard.whitePawns : chessboard.blackPawns;
    while (fromSquares != 0) {
        unsigned short fromSquareIndex = POP_LSB(fromSquares);
        Bitboard fromSquare = BITBOARD(fromSquareIndex);
        Bitboard toSquares = (chessboard.turn == White) ? whitePawnAttacks[fromSquareIndex] : blackPawnAttacks[fromSquareIndex];

        // Consider conditional moves
        if (chessboard.turn == White) {
            // Check for initial double advance conditions
            if ((fromSquare & RANK_2) != 0 && (((fromSquare << 8) | (fromSquare << 16)) & chessboard.allPieces) == 0)
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, (fromSquare << 16), Move::DoublePawnPush));
            // Check for diagonal pieces to capture
            if ((northeast(fromSquare) & chessboard.blackPieces) != 0)
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, northeast(fromSquare), Move::Capture));
            if ((northwest(fromSquare) & chessboard.blackPieces) != 0)
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, northwest(fromSquare), Move::Capture));
            // Check for en passant
            if (chessboard.enPassant == east(fromSquare))
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, northeast(fromSquare), Move::EnPassant));
            if (chessboard.enPassant == west(fromSquare))
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, northwest(fromSquare), Move::EnPassant));
        } else {
            // Check for initial double advance conditions
            if ((fromSquare & RANK_7) != 0 && (((fromSquare >> 8) | (fromSquare >> 16)) & chessboard.allPieces) == 0)
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, (fromSquare >> 16), Move::DoublePawnPush));
            // Check for diagonal pieces to capture
            if ((southeast(fromSquare) & chessboard.whitePieces) != 0)
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, southeast(fromSquare), Move::Capture));
            if ((southwest(fromSquare) & chessboard.whitePieces) != 0)
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, southwest(fromSquare), Move::Capture));
            // Check for en passant
            if (chessboard.enPassant == east(fromSquare))
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, southeast(fromSquare), Move::EnPassant));
            if (chessboard.enPassant == west(fromSquare))
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, southwest(fromSquare), Move::EnPassant));
        }

        // Add normal advances
        while (toSquares != 0) {
            unsigned short toSquare = POP_LSB(toSquares);
            chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, Move::Quiet));
        }
    }
}



// Change move generation functions to take parameter of movelist to append to and have this generate a movelist from that
MoveList Chessboard::generatePseudoLegalMoves() {
    
}

MoveList Chessboard::generateLegalMoves() {
    MoveList pseudoLegalMoves = generatePseudoLegalMoves();
}