#include "move.h"
#include <iostream>

Move::Move() {
    // Represents the null move, quiet and does not change board state
    this->move = 0;
}

Move::Move(unsigned short fromSquare, unsigned short toSquare, unsigned short moveType) {
    this->move = (moveType << 12) | (toSquare << 6) | (fromSquare);
}

Square Move::getFromSquare() { return static_cast<Square>(this->move & 0x3F); }
Square Move::getToSquare() { return static_cast<Square>((this->move >> 6) & 0x3F); }
Move::MoveType Move::getMoveType() { return static_cast<MoveType>((this->move >> 12) & 0xF); }
bool Move::isQuiet() { return !(static_cast<bool>(this->move >> 12)); } // Inverted because quiet is 0b0000, which is false
bool Move::isCapture() { return static_cast<bool>(this->move & (1 << 15) != 0); }
bool Move::isPromotion() { return static_cast<bool>(this->move & (1 << 14) != 0); }
void Move::printMove() { std::cout << this->getFromSquare() << " " << this->getToSquare() << std::endl; }