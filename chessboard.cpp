#include "chessboard.h"
#include <iostream>
#include "board_visualization.h"

Chessboard::Chessboard() {
    // Initialize the bitboards to match the piece's positions at the start of the game
    whitePawns = 0xFF00ULL;
    whiteKnights = 0x42ULL;
    whiteBishops = 0x24ULL;
    whiteRooks = 0x81ULL;
    whiteQueen = 0x10ULL;
    whiteKing = 0x08ULL;
    whitePieces = 0xFFFFULL;

    blackPawns = 0xFF000000000000ULL;
    blackKnights = 0x4200000000000000ULL;
    blackBishops = 0x2400000000000000ULL;
    blackRooks = 0x8100000000000000ULL;
    blackQueen = 0x1000000000000000ULL;
    blackKing = 0x0800000000000000ULL;
    blackPieces = 0xFFFF000000000000ULL;

    allPieces = whitePieces | blackPieces;

    turn = White;

    // Negative value used to indicate no possible en passant moves as it does not correspond to any square index
    Bitboard enPassant = 0ULL;

    // Players start out with all castling rights
    whiteQueenCastle = whiteKingCastle = blackQueenCastle = blackKingCastle = true;
}

// Return the type of piece present at a given square
PieceType Chessboard::pieceAt(unsigned short square) {
    if (GET_BIT(whitePawns, square) != 0 || GET_BIT(blackPawns, square) != 0) {
        return PieceType::Pawn;
    } else if (GET_BIT(whiteKnights, square) != 0 || GET_BIT(blackKnights, square) != 0) {
        return PieceType::Knight;
    } else if (GET_BIT(whiteBishops, square) != 0 || GET_BIT(blackBishops, square) != 0) {
        return PieceType::Bishop;
    } else if (GET_BIT(whiteRooks, square) != 0 || GET_BIT(blackRooks, square) != 0) {
        return PieceType::Rook;
    } else if (GET_BIT(whiteQueen, square) != 0 || GET_BIT(blackQueen, square) != 0) {
        return PieceType::Queen;
    } else if (GET_BIT(whiteKing, square) != 0 || GET_BIT(blackKing, square) != 0) {
        return PieceType::King;
    } else {
        return PieceType::None;
    }
}

// Push a move onto the board
void Chessboard::push(Move move) {
    pastMoves.push_back(move);
    Square fromSquare = move.getFromSquare(), toSquare = move.getToSquare();
    PieceType fromPiece = pieceAt(fromSquare);
    PieceType toPiece = (move.isCapture()) ? pieceAt(toSquare) : PieceType::None;

    Bitboard *movingBoard;
    switch (fromPiece) {
        case PieceType::Pawn:
            movingBoard = (turn == White) ? &whitePawns : &blackPawns;
            break;
        case PieceType::Knight:
            movingBoard = (turn == White) ? &whiteKnights : &blackKnights;
            break;
        case PieceType::Bishop:
            movingBoard = (turn == White) ? &whiteBishops : &blackBishops;
            break;
        case PieceType::Rook:
            movingBoard = (turn == White) ? &whiteRooks : &blackRooks;
            break;
        case PieceType::Queen:
            movingBoard = (turn == White) ? &whiteQueen : &blackQueen;
            break;
        case PieceType::King:
            movingBoard = (turn == White) ? &whiteKing : &blackKing;
            break;
    }

    Bitboard *captureBoard = (turn == White) ? &blackPieces : &whitePieces; // Will get cleared regardless
    switch (toPiece) {
        case PieceType::None:
            break;
        case PieceType::Pawn:
            captureBoard = (turn == White) ? &blackPawns : &whitePawns;
        case PieceType::Knight:
            captureBoard = (turn == White) ? &blackKnights : &whiteKnights;
            break;
        case PieceType::Bishop:
            captureBoard = (turn == White) ? &blackBishops : &whiteBishops;
            break;
        case PieceType::Rook:
            captureBoard = (turn == White) ? &blackRooks : &whiteRooks;
            break;
        case PieceType::Queen:
            captureBoard = (turn == White) ? &blackQueen : &whiteQueen;
            break;
        case PieceType::King:
            captureBoard = (turn == White) ? &blackKing : &whiteKing;
            break;
    }

    // Update specific piece type boards
    CLEAR_BIT(*movingBoard, fromSquare);
    SET_BIT(*movingBoard, toSquare);
    CLEAR_BIT(*captureBoard, toSquare);

    // Update boards for all piece types
    if (turn == White) {
        CLEAR_BIT(whitePieces, fromSquare);
        SET_BIT(whitePieces, toSquare);
        CLEAR_BIT(blackPieces, toSquare);
    } else {
        CLEAR_BIT(blackPieces, fromSquare);
        SET_BIT(blackPieces, toSquare);
        CLEAR_BIT(whitePieces, toSquare);
    }
}