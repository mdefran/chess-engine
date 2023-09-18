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

    // Tracks moves made so far
    MoveList pastMoves;

    // Variable to store double pawn moves that could enable an en passant
    Bitboard enPassant;

    // Used to track castling rights for each side
    bool whiteQueenCastle, whiteKingCastle, blackQueenCastle, blackKingCastle;

    // Functions to keep track of the previous move's data for the pop function
    bool whiteQueenCastleBeforeMove, whiteKingCastleBeforeMove, blackQueenCastleBeforeMove, blackKingCastleBeforeMove;
    std::vector<PieceType> capturedPieces;

    // Constructor for the start of the game
    Chessboard();

    // Move generation
    // Generate all legal moves for current player
    MoveList generateLegalMoves();
    // Generate possible moves not considering check, ally piece placement, etc
    MoveList generatePseudoLegalMoves();

    // Square info
    // Returns the piece type at a given square
    PieceType pieceAt(Square square);
    // Returns whether or not a given square is under attack by the opponent
    bool underAttack(Square square);

    // Board manipulation
    // Play a move to the board
    void push(Move move);
    // Undo the last move made
    void pop();

    // Endgame detection
    bool isCheck();
    bool isCheckmate();
    Color winningSide();
    bool isStalemate();
};

#endif // CHESSBOARD_H