#include <unordered_map>
#include "chessboard.h"
#include "move.h"
#include "bitboard.h"
#include "types.h"
#include "magic_bitboards.h"

// Declare lookup tables for leaping pieces
Bitboard knightAttacks[64], kingAttacks[64], whitePawnAttacks[64], blackPawnAttacks[64];
// Declare lookup tables for sliding pieces
std::unordered_map<int, Bitboard> bishopAttacks, rookAttacks;
Bitboard rookMasks[64], bishopMasks[64];

const int rookRelevantBits[64] {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
};
const int bishopRelevantBits[64] {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};

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

        rookMasks[square] = 0ULL;
        for (int bit = square / 8; bit < 7; bit++) rookMasks[square] |= fromSquare << (bit * 8); // North
        for (int bit = square / 8; bit > 1; bit--) rookMasks[square] |= fromSquare >> (bit * 8); // South
        for (int bit = square % 8; bit > 1; bit--) rookMasks[square] |= fromSquare >> bit; // East
        for (int bit = square % 8; bit < 7; bit++) rookMasks[square] |= fromSquare << bit; // West

        bishopMasks[square] = 0ULL;
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
        // Check if the move is a capture or not
        Move::MoveType type = (toSquare & ((chessboard.turn == White) ? chessboard.blackPieces : chessboard.whitePieces) != 0) ? Move::Capture : Move::Quiet;
        if (toSquare & ((chessboard.turn == White) ? chessboard.whitePieces : chessboard.blackPieces) == 0)
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
            // Check if the move is a capture or not
            Move::MoveType type = (toSquare & ((chessboard.turn == White) ? chessboard.blackPieces : chessboard.whitePieces) != 0) ? Move::Capture : Move::Quiet;
            // Destination square should not be occupied by ally piece
            if (toSquare & ((chessboard.turn == White) ? chessboard.whitePieces : chessboard.blackPieces) == 0)
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, type));
        }
    }
}

void pushPromotionCapture(Chessboard &chessboard, Bitboard fromSquare, Bitboard toSquare) {
    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, Move::KnightPromotionCapture));
    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, Move::BishopPromotionCapture));
    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, Move::RookPromotionCapture));
    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, Move::QueenPromotionCapture));
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
                // Check for promotion captures
                if ((north(fromSquare) & RANK_8) != 0)
                    pushPromotionCapture(chessboard, fromSquare, northeast(fromSquare));
                else
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, northeast(fromSquare), Move::Capture));
            if ((northwest(fromSquare) & chessboard.blackPieces) != 0)
                // Check for promotion captures
                if ((north(fromSquare) & RANK_8) != 0)
                    pushPromotionCapture(chessboard, fromSquare, northwest(fromSquare));
                else
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, northwest(fromSquare), Move::Capture));
            // Check for en passant
            if (chessboard.enPassant == east(fromSquare))
                // Destination square should not be occupied by ally piece
                if (northeast(fromSquare) & ((chessboard.turn == White) ? chessboard.whitePieces : chessboard.blackPieces) == 0)
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, northeast(fromSquare), Move::EnPassant));
            if (chessboard.enPassant == west(fromSquare))
                // Destination square should not be occupied by ally piece
                if (northwest(fromSquare) & ((chessboard.turn == White) ? chessboard.whitePieces : chessboard.blackPieces) == 0)
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, northwest(fromSquare), Move::EnPassant));
        } else {
            // Check for initial double advance conditions
            if ((fromSquare & RANK_7) != 0 && (((fromSquare >> 8) | (fromSquare >> 16)) & chessboard.allPieces) == 0)
                chessboard.pseudoLegalMoves.push_back(Move(fromSquare, (fromSquare >> 16), Move::DoublePawnPush));
            // Check for diagonal pieces to capture
            if ((southeast(fromSquare) & chessboard.whitePieces) != 0)
                // Check for promotion captures
                if ((south(fromSquare) & RANK_1) != 0)
                    pushPromotionCapture(chessboard, fromSquare, southeast(fromSquare));
                else
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, southeast(fromSquare), Move::Capture));
            if ((southwest(fromSquare) & chessboard.whitePieces) != 0)
                // Check for promotion captures
                if ((south(fromSquare) & RANK_1) != 0)
                    pushPromotionCapture(chessboard, fromSquare, southwest(fromSquare));
                else
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, southwest(fromSquare), Move::Capture));
            // Check for en passant
            if (chessboard.enPassant == east(fromSquare))
                // Destination square should not be occupied by ally piece
                if (southeast(fromSquare) & ((chessboard.turn == White) ? chessboard.whitePieces : chessboard.blackPieces) == 0)
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, southeast(fromSquare), Move::EnPassant));
            if (chessboard.enPassant == west(fromSquare))
                // Destination square should not be occupied by ally piece
                if (southwest(fromSquare) & ((chessboard.turn == White) ? chessboard.whitePieces : chessboard.blackPieces) == 0)
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, southwest(fromSquare), Move::EnPassant));
        }

        // Add normal advances
        while (toSquares != 0) {
            unsigned short toSquare = POP_LSB(toSquares);

            // Add normal promotions
            if ((toSquare & RANK_8 != 0) || (toSquare & RANK_1 != 0)) {
                // Destination square should not be occupied by ally piece
                if (toSquare & ((chessboard.turn == White) ? chessboard.whitePieces : chessboard.blackPieces) == 0) {
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, Move::KnightPromotion));
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, Move::BishopPromotion));
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, Move::RookPromotion));
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, Move::QueenPromotion));
                }
            } else {
                // Destination square should not be occupied by ally piece
                if (toSquare & ((chessboard.turn == White) ? chessboard.whitePieces : chessboard.blackPieces) == 0)
                    chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, Move::Quiet));
            }
        }
    }
}

void generateRookAttacks(Chessboard &chessboard) {

}

void generateBishopAttacks(Chessboard &chessboard) {

}

// Change move generation functions to take parameter of movelist to append to and have this generate a movelist from that
MoveList Chessboard::generatePseudoLegalMoves() {
    
}

MoveList Chessboard::generateLegalMoves() {
    MoveList pseudoLegalMoves = generatePseudoLegalMoves();
}