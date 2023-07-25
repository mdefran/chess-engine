#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include "chess.cpp"

class Move {
private:
    /*
    Each move is encoded into a 16-bit variable.
    Starting from the rightmost bit:
    Bits 0-5 represent the square index of the origin square, 0-63.
    Bits 6-11 represent the square index of the destination square, 0-63.
    The remaining bits, 12-15, are used to indicate the type of the move.
    */
    uint16_t move;

public:
    Move();
    Move(unsigned short fromSquare, unsigned short toSquare, unsigned short moveType);

    /*
    The 4 most significant bits represent the move type.
    Starting from the rightmost bit:
    Bits 0-1 are used purely to differentiate between the types.
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

    Square getFromSquare();
    Square getToSquare();
    MoveType getMoveType();
    bool isQuiet();
    bool isCapture();
    bool isPromotion();
};

#endif // MOVE_H