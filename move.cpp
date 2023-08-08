#include "move.h"
#include "bitboard.h"
#include <iostream>

// Constructors for null, squares, and bitboards
Move::Move() {this->move = 0; } // Represents the null move, quiet and does not change board state
Move::Move(Square fromSquare, Square toSquare, MoveType moveType) { this->move = (moveType << 12) | (static_cast<unsigned short>(toSquare << 6)) | (static_cast<unsigned short>(fromSquare)); }
Move::Move(Bitboard fromSquare, Bitboard toSquare, MoveType moveType) { this->move = (moveType << 12) | (GET_LSB(toSquare << 6)) | (GET_LSB(fromSquare)); }

// Getter functions
Square Move::getFromSquare() {return static_cast<Square>(this->move & 0x3F); }
Square Move::getToSquare() { return static_cast<Square>((this->move >> 6) & 0x3F); }
Move::MoveType Move::getMoveType() { return static_cast<MoveType>((this->move >> 12) & 0xF); }
bool Move::isQuiet() { return !(static_cast<bool>(this->move >> 12)); } // Inverted because quiet is 0b0000, which is false
bool Move::isCapture() { return static_cast<bool>(this->move & (1 << 15) != 0); }
bool Move::isPromotion() { return static_cast<bool>(this->move & (1 << 14) != 0); }
bool Move::isNull() { return !(this->move); }

// Display functions
void Move::printMove() { std::cout << this->getFromSquare() << " " << this->getToSquare() << std::endl; }