#include "chessboard.h"

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

PieceType Chessboard::pieceAt(Square square) {
    if (turn == White) {
        if (GET_BIT(whitePawns, square) != 0) {
            return PieceType::Pawn;
        } else if (GET_BIT(whiteKnights, square) != 0) {
            return PieceType::Knight;
        } else if (GET_BIT(whiteBishops, square) != 0) {
            return PieceType::Bishop;
        } else if (GET_BIT(whiteQueen, square) != 0) {
            return PieceType::Queen;
        } else if (GET_BIT(whiteKing, square) != 0) {
            return PieceType::King;
        } else {
            return PieceType::None;
        }
    } else {
        if (GET_BIT(blackPawns, square) != 0) {
            return PieceType::Pawn;
        } else if (GET_BIT(blackKnights, square) != 0) {
            return PieceType::Knight;
        } else if (GET_BIT(blackBishops, square) != 0) {
            return PieceType::Bishop;
        } else if (GET_BIT(blackQueen, square) != 0) {
            return PieceType::Queen;
        } else if (GET_BIT(blackKing, square) != 0) {
            return PieceType::King;
        } else {
            return PieceType::None;
        }
    }
}

void Chessboard::push(Move move) {
    pastMoves.push_back(move);
    Square fromSquare = move.getFromSquare(), toSquare = move.getToSquare();
}

void Chessboard::pop() {

}

Move Chessboard::peek() {

}

bool Chessboard::isCheck() {
    
}

bool Chessboard::isCheckmate() {

}

Color Chessboard::winningSide() {

}