#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "bitboard.h"
#include "move.h"
#include "types.h"

struct Chessboard {
    // Bitboards representing piece locations
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

    // Active player
    Color turn;
    
    // Declare lookup tables for leaping pieces
    Bitboard knightAttacks[64], kingAttacks[64], whitePawnAttacks[64], blackPawnAttacks[64];

    // Move lists for move generation and tracking
    MoveList pseudoLegalMoves, legalMoves, pastMoves;

    // Variable to store double pawn moves that could enable an en passant
    Bitboard enPassant;

    // Used to track castling rights for each side
    bool whiteQueenCastle, whiteKingCastle, blackQueenCastle, blackKingCastle;

    // Constructor for the start of the game
    Chessboard();

    // Move generation
    MoveList generatePseudoLegalMoves(); // Generate possible moves not considering check, ally piece placement, etc
    MoveList generateLegalMoves(); // Generate only validated pseudo legal moves

    // Move manipulation
    void push(Move move); // Play a move to the board
    void pop(); // Undo the last move made
    Move peek(); // View the last move made

    // Game ending detection
    bool isCheck();
    bool isCheckmate();
    Color winningSide();
    bool isStalemate();
};

#endif // CHESSBOARD_H