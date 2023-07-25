#include "chessboard.h"
#include "move.h"
#include "bitboard.h"
#include "types.h"

// Generate pseudo legal king moves for the current player using the attack maps
void generateKingMoves(Chessboard &chessboard) {
    unsigned short fromSquareIndex = (chessboard.turn == White) ? POP_LSB(chessboard.whiteKing) : POP_LSB(chessboard.blackKing);
    Bitboard fromSquare = BITBOARD(fromSquareIndex);
    Bitboard toSquares = chessboard.kingAttacks[fromSquareIndex];

    // Check for pseudo legal castling
    Move move;
    if (chessboard.turn == White) {
        if (chessboard.whiteKingCastle == true && (chessboard.allPieces & (0x6ULL)) == 0) {
            move = Move(3, 0, Move::KingCastle);
            chessboard.pseudoLegalMoves.push_back(move);
        }
        if (chessboard.whiteQueenCastle == true && (chessboard.allPieces & (0x70ULL)) == 0) {
            move = Move(3, 7, Move::QueenCastle);
            chessboard.pseudoLegalMoves.push_back(move);
        }
    } else {
        if (chessboard.blackKingCastle == true && (chessboard.allPieces & (0x600000000000000ULL)) == 0) {
            move = Move(59, 56, Move::KingCastle);
            chessboard.pseudoLegalMoves.push_back(move);
        }
        if (chessboard.blackQueenCastle == true && (chessboard.allPieces & (0x7000000000000000) == 0)) {
            move = Move(59, 63, Move::QueenCastle);
            chessboard.pseudoLegalMoves.push_back(move);
        }
    }

    while (toSquares != 0) {
        unsigned short toSquare = POP_LSB(toSquares);
        Move::MoveType type = (toSquare & ((chessboard.turn == White) ? chessboard.blackPieces : chessboard.whitePieces) != 0) ? Move::Capture : Move::Quiet;
        move = Move(fromSquareIndex, toSquare, type);
        chessboard.pseudoLegalMoves.push_back(move);
    }
}

// Generate pseudo legal knight moves for the current player using the attack maps
void generateKnightMoves(Chessboard &chessboard) {
    Bitboard fromSquares = (chessboard.turn == White) ? chessboard.whiteKnights : chessboard.blackKnights;
    while (fromSquares != 0) {
        unsigned short fromSquare = POP_LSB(fromSquares);
        Bitboard toSquares = chessboard.knightAttacks[fromSquare];

        while (toSquares != 0) {
            addMove(fromSquare, POP_LSB(toSquares));
        }
    }
}

void generatePawnMoves(Chessboard &chessboard) {
    Bitboard fromSquares = (chessboard.turn == White) ? chessboard.whitePawns : chessboard.blackPawns;
    while (fromSquares != 0) {
        unsigned short fromSquareIndex = POP_LSB(fromSquares);
        Bitboard fromSquare = BITBOARD(fromSquareIndex);
        Bitboard toSquares = (chessboard.turn == White) ? chessboard.whitePawnAttacks[fromSquareIndex] : chessboard.blackPawnAttacks[fromSquareIndex];

        // Consider conditional moves
        if (turn == White) {
            // Check for initial double advance conditions
            if ((fromSquare & RANK_2) != 0 && (((fromSquare << 8) | (fromSquare << 16)) & chessboard.allPieces) == 0)
                toSquares |= fromSquare << 16;
            // Check for diagonal pieces to capture
            if ((northeast(fromSquare) & chessboard.blackPieces) != 0)
                toSquares |= northeast(fromSquare);
            if ((northwest(fromSquare) & chessboard.blackPieces) != 0)
                toSquares |= northwest(fromSquare);
        } else {
            // Check for initial double advance conditions
            if ((fromSquare & RANK_7) != 0 && (((fromSquare >> 8) | (fromSquare >> 16)) & chessboard.allPieces) == 0)
                toSquares |= fromSquare >> 16;
            // Check for diagonal pieces to capture
            if ((southeast(fromSquare) & chessboard.whitePieces) != 0)
                toSquares |= southeast(fromSquare);
            if ((southwest(fromSquare) & chessboard.whitePieces) != 0)
                toSquares |= southwest(fromSquare);
        }

        // Add the possible moves
        while (toSquares != 0) {
            addMove(fromSquareIndex, POP_LSB(toSquares));
        }
    }
}