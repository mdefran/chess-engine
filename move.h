#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include <vector>
#include <string>
#include "bitboard.h"
#include "types.h"

class Move {
private:
    /*
    Each move is encoded into a 16-bit variable.
    Starting from the rightmost bit:
    Bits 0-5 represent the index of the origin square, 0-63.
    Bits 6-11 represent the index of the destination square, 0-63.
    The remaining bits, 12-15, are used to indicate the type of the move.
    */
    uint16_t move;

public:
    /*
    The 4 most significant bits represent the move type.
    Starting from the rightmost of those bits:
    Bits 0-1 are used purely to differentiate between the types and have no other meanings.
    Bit 2 indicates whether or not the move is a promotion.
    Bit 3 indicates whether or not the move is a capture.
    */
    enum MoveType : unsigned short {
        Quiet = 0, // 0b0000
        Capture = 8, // 0b1000
        EnPassant = 10, // 0b1010
        DoublePawnPush = 1, // 0b0001
        KingCastle = 2, // 0b0010
        QueenCastle = 3, // 0b0011
        KnightPromotion = 4, // 0b0100
        BishopPromotion = 5, // 0b0101
        RookPromotion = 6, // 0b0110
        QueenPromotion = 7, // 0b0111
        KnightPromotionCapture = 12, // 0b1100
        BishopPromotionCapture = 13, // 0b1101
        RookPromotionCapture = 14, // 0b1110
        QueenPromotionCapture = 15 // 0b1111
    };

    // Index-based square names for move output
    std::string squareNames[64] = {
        "h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1",
        "h2", "g2", "f2", "e2", "d2", "c2", "b2", "a2",
        "h3", "g3", "f3", "e3", "d3", "c3", "b3", "a3",
        "h4", "g4", "f4", "e4", "d4", "c4", "b4", "a4",
        "h5", "g5", "f5", "e5", "d5", "c5", "b5", "a5",
        "h6", "g6", "f6", "e6", "d6", "c6", "b6", "a6",
        "h7", "g7", "f7", "e7", "d7", "c7", "b7", "a7",
        "h8", "g8", "f8", "e8", "d8", "c8", "b8", "a8"
    };

    // Constructors
    Move();
    Move(Square fromSquare, Square toSquare, MoveType moveType);
    Move(Bitboard fromSquare, Bitboard toSquare, MoveType moveType);

    // Getter functions
    Square getFromSquare();
    Square getToSquare();
    MoveType getMoveType();
    bool isQuiet();
    bool isCapture();
    bool isPromotion();
    bool isNull();

    // Display functions
    void printMove();
};

typedef std::vector<Move> MoveList;

#endif // MOVE_H