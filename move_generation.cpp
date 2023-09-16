#include <unordered_map>
#include "chessboard.h"
#include "move.h"
#include "bitboard.h"
#include "types.h"
#include "board_visualization.h"

// Declare lookup tables for leaping pieces
Bitboard knightAttacks[64], kingAttacks[64], whitePawnAttacks[64], blackPawnAttacks[64], whitePawnCaptures[64], blackPawnCaptures[64];
// Declare sliding piece run-time attack boards
Bitboard rookAttacks, bishopAttacks;

void initializeLookupTables() {
    // Loop through each square individually
    for (int square = 0; square < 64; square++) {
        // Get square information
        Bitboard fromSquare = BITBOARD(square);
        int rank = square / 8;
        int file = square % 8; // Reversed from traditional chess files due to endianness of squares

        // Kings can attack in any adjacent square
        kingAttacks[square] = north(fromSquare) | northeast(fromSquare) | east(fromSquare) | 
                              southeast(fromSquare) | south(fromSquare) | southwest(fromSquare) |
                              west(fromSquare) | northwest(fromSquare);

        // Knights can attack in an L shape
        knightAttacks[square] = (((fromSquare & ~(FILE_G | FILE_H | RANK_8)) << 6) | ((fromSquare & ~(FILE_G | FILE_H | RANK_1)) >> 10)) |
                                (((fromSquare & ~(FILE_H | RANK_7 | RANK_8)) << 15) | ((fromSquare & ~(FILE_H | RANK_1 | RANK_2)) >> 17)) |
                                (((fromSquare & ~(FILE_A | RANK_7 | RANK_8)) << 17) | ((fromSquare & ~(FILE_A | RANK_1 | RANK_2)) >> 15)) |
                                (((fromSquare & ~(FILE_A | FILE_B | RANK_8)) << 10) | ((fromSquare & ~(FILE_A | FILE_B | RANK_1)) >> 6));

        // Pawns can advance forward (captures and special moves handled in generation)
        whitePawnAttacks[square] = north(fromSquare);
        blackPawnAttacks[square] = south(fromSquare);
        whitePawnCaptures[square] = northeast(fromSquare) | northwest(fromSquare);
        blackPawnCaptures[square] = southeast(fromSquare) | southwest(fromSquare);
    }
}

// Pushes moves to the pseudo legal move list, marking captures and ignoring friendly fire
void pushPseudoLegalMove(Chessboard &chessboard, Square fromSquare, Square toSquare) {
    // Set the move type to capture if the destination square is occupied by an enemy piece; otherwise, default to quiet
    Move::MoveType type = (BITBOARD(toSquare) & ((chessboard.turn == White) ? chessboard.blackPieces : chessboard.whitePieces) != 0) ? Move::Capture : Move::Quiet;
    // If the destination square is occupied by an ally piece, do not push the move; otherwise, push it to the vector      
    if ((BITBOARD(toSquare) & ((chessboard.turn == White) ? chessboard.whitePieces : chessboard.blackPieces)) == 0)
        chessboard.pseudoLegalMoves.push_back(Move(fromSquare, toSquare, type));
}

void generateKingMoves(Chessboard &chessboard) {
    Square fromSquare = (chessboard.turn == White) ? static_cast<Square>(POP_LSB(chessboard.whiteKing)) : static_cast<Square>(POP_LSB(chessboard.blackKing));
    Bitboard toSquares = kingAttacks[fromSquare];
    
    Move move;
    // Check for pseudo legal castling
    if (chessboard.turn == White) {
        if (chessboard.whiteKingCastle == true && (chessboard.allPieces & (0x6ULL)) == 0)
            chessboard.pseudoLegalMoves.push_back(Move(Square::e1, Square::g1, Move::KingCastle));
        if (chessboard.whiteQueenCastle == true && (chessboard.allPieces & (0x70ULL)) == 0)
            chessboard.pseudoLegalMoves.push_back(Move(Square::e1, Square::c1, Move::QueenCastle));
    } else {
        if (chessboard.blackKingCastle == true && (chessboard.allPieces & (0x600000000000000ULL)) == 0)
            chessboard.pseudoLegalMoves.push_back(Move(Square::e8, Square::g8, Move::KingCastle));
        if (chessboard.blackQueenCastle == true && (chessboard.allPieces & (0x7000000000000000) == 0))
            chessboard.pseudoLegalMoves.push_back(Move(Square::e8, Square::c8, Move::QueenCastle));
    }

    while (toSquares != 0) {
        Square toSquare = static_cast<Square>(POP_LSB(toSquares));
        pushPseudoLegalMove(chessboard, fromSquare, toSquare);
    }
}

// Generate pseudo legal knight moves for the current player using the attack maps
void generateKnightMoves(Chessboard &chessboard) {
    Bitboard fromSquares = (chessboard.turn == White) ? chessboard.whiteKnights : chessboard.blackKnights;
    while (fromSquares != 0) {
        Square fromSquare = static_cast<Square>(POP_LSB(fromSquares));
        Bitboard toSquares = knightAttacks[fromSquare];

        Move move;
        while (toSquares != 0) {
            Square toSquare = static_cast<Square>(POP_LSB(toSquares));
            pushPseudoLegalMove(chessboard, fromSquare, toSquare);
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

void generateRookMoves(Chessboard &chessboard) {
    Bitboard fromSquares = (chessboard.turn == White) ? chessboard.whiteRooks : chessboard.blackRooks;
    Bitboard toSquares = 0ULL;

    // Generate potential moves
    while (fromSquares != 0) {
        // Get starting square information
        Square fromSquare = static_cast<Square>(POP_LSB(fromSquares));
        int rank = fromSquare / 8;
        int file = fromSquare % 8; // Reversed from traditional chess files due to endianness of squares

        // Generate potential squares to move to
        Bitboard toSquare = 0ULL;

        // North
        for (int r = rank + 1; r < 8; r++) {
            toSquare = BITBOARD(r * 8 + file);
            toSquares |= toSquare;
            // Stop once a piece is hit
            if (chessboard.allPieces & toSquare)
                break;
        }

        // South
        for (int r = rank - 1; r >= 0; r--) {
            toSquare = BITBOARD(r * 8 + file);
            toSquares |= toSquare;
            // Stop once a piece is hit
            if (chessboard.allPieces & toSquare)
                break;
        }

        // East
        for (int f = file - 1; f >= 0; f--) {
            toSquare = BITBOARD(rank * 8 + f);
            toSquares |= toSquare;
            // Stop once a piece is hit
            if (chessboard.allPieces & toSquare)
                break;
        }

        // West
        for (int f = file + 1; f < 8; f++) {
            toSquare = BITBOARD(rank * 8 + f);
            toSquares |= toSquare;
            // Stop once a piece is hit
            if (chessboard.allPieces & toSquare)
                break;
        }

        // Push pseudo legal move
        Move move;
        while (toSquares != 0) {
            Square toSquare = static_cast<Square>(POP_LSB(toSquares));
            pushPseudoLegalMove(chessboard, fromSquare, toSquare);
        }
    }
}

void generateBishopMoves(Chessboard &chessboard) {
    Bitboard fromSquares = (chessboard.turn == White) ? chessboard.whiteBishops : chessboard.blackBishops;
    Bitboard toSquares = 0ULL;

    // Generate potential moves
    while (fromSquares != 0) {
        // Get starting square information
        Square fromSquare = static_cast<Square>(POP_LSB(fromSquares));
        int rank = fromSquare / 8;
        int file = fromSquare % 8; // Reversed from traditional chess files due to endianness of squares

        // Generate potential squares to move to
        Bitboard toSquare = 0ULL;

        // Northeast
        for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
            toSquare = BITBOARD(r * 8 + f);
            toSquares |= toSquare;
            // Stop once a piece is hit
            if (chessboard.allPieces & toSquare)
                break;
        }

        // Southeast
        for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
            toSquare = BITBOARD(r * 8 + file);
            toSquares |= toSquare;
            // Stop once a piece is hit
            if (chessboard.allPieces & toSquare)
                break;
        }

        // Southwest
        for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
            toSquare = BITBOARD(r * 8 + f);
            toSquares |= toSquare;
            // Stop once a piece is hit
            if (chessboard.allPieces & toSquare)
                break;
        }

        // Northwest
        for (int r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
            toSquare = BITBOARD(r * 8 + f);
            toSquares |= toSquare;
            // Stop once a piece is hit
            if (chessboard.allPieces & toSquare)
                break;
        }

        // Push pseudo legal move
        Move move;
        while (toSquares != 0) {
            Square toSquare = static_cast<Square>(POP_LSB(toSquares));
            pushPseudoLegalMove(chessboard, fromSquare, toSquare);
        }
    }
}

/*
void Chessboard::generatePseudoLegalMoves() {
    //generatePawnMoves(*this);
    //generateKnightMoves(*this);
    //generateRookMoves(*this);
    //generateBishopMoves(*this);
    //generateQueenMoves(*this);
    //generateKingMoves(*this);
}

MoveList Chessboard::generateLegalMoves() {
    MoveList pseudoLegalMoves = generatePseudoLegalMoves();
}
*/

int main () {
    Chessboard chessboard;
    initializeLookupTables();
    
    while (!chessboard.pseudoLegalMoves.empty()) {
        Move move = chessboard.pseudoLegalMoves.back();
        move.printMove();
        chessboard.pseudoLegalMoves.pop_back();
    }

    return 0;
}