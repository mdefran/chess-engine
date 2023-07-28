#include "chessboard.h"
#include <iostream>
#include "board_visualization.h"
#include "move.h"

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
    // Gather move information
    pastMoves.push_back(move);
    Square fromSquare = move.getFromSquare(), toSquare = move.getToSquare();
    Move::MoveType moveType = move.getMoveType();
    PieceType fromPiece = pieceAt(fromSquare);
    PieceType toPiece = (move.isCapture()) ? pieceAt(toSquare) : PieceType::None;

    // If a player's king moves from the starting position, disable castling for them
    if (fromPiece == PieceType::King) {
        if (turn == White && fromSquare == Square::e1) {
            whiteKingCastle = whiteQueenCastle = false;
        } else if (turn == Black && fromSquare == Square::e8) {
            blackKingCastle = blackQueenCastle = false;
        }
    }

    // If a rook moves, disable castling for that corner
    if (fromSquare == Square::h1)
        whiteKingCastle = false;
    if (fromSquare == Square::a1)
        whiteQueenCastle = false;
    if (fromSquare == Square::h8)
        blackKingCastle = false;
    if (fromSquare == Square::a8)
        blackQueenCastle = false;

    // Handle extra rook movement for castling
    if (turn == White && moveType == Move::KingCastle) {
        whiteKingCastle = false;
        this->push(Move(Square::h1, Square::f1, Move::Quiet));
        this->pop(); // Remove the extra move just logged as castling is recorded as one move
    } else if (turn == White && moveType == Move::QueenCastle) {
        whiteQueenCastle = false;
        this->push(Move(Square::a1, Square::d1, Move::Quiet));
        this->pop();
    } else if (turn == Black && moveType == Move::KingCastle) {
        blackKingCastle == false;
        this->push(Move(Square::h8, Square::f8, Move::Quiet));
        this->pop();
    } else if (turn == Black && moveType == Move::QueenCastle) {
        blackQueenCastle == false;
        this->push(Move(Square::a8, Square::d8, Move::Quiet));
        this->pop();
    }

    // Store en passant squares if necessary
    if (moveType == Move::DoublePawnPush)
        enPassant = move.getToSquare();

    // Perform en passant pawn capture
    if (moveType = Move::EnPassant) {
        if (BITBOARD(toSquare) == northeast(BITBOARD(fromSquare))) {
            unsigned short captureSquare = GET_LSB(south(BITBOARD(toSquare)));
            // We know the captured pawn is black since the advancing pawn is moving north
            CLEAR_BIT(blackPawns, captureSquare);
        } else if (BITBOARD(toSquare) == northwest(BITBOARD(fromSquare))) {
            unsigned short captureSquare = GET_LSB(south(BITBOARD(toSquare)));
            CLEAR_BIT(blackPawns, captureSquare);
        } else if (BITBOARD(toSquare) == southeast(BITBOARD(fromSquare))) {
            unsigned short captureSquare = GET_LSB(north(BITBOARD(toSquare)));
            // We know the captured pawn is white since the advanced pawn is moving south
            CLEAR_BIT(whitePawns, captureSquare);
        } else {
            unsigned short captureSquare = GET_LSB(north(BITBOARD(toSquare)));
            CLEAR_BIT(whitePawns, captureSquare);
        }
    }

    // Move piece on its board
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
    CLEAR_BIT(*movingBoard, fromSquare);
    SET_BIT(*movingBoard, toSquare);

    // Erase pieces at capture positions
    if (move.isCapture()) {
        Bitboard *captureBoard;
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

        CLEAR_BIT(*captureBoard, toSquare);
    }

    // Perform promotions
    if (move.isPromotion()) {
        Bitboard *promotionBoard;
        switch (moveType) {
            case Move::KnightPromotion: case Move::KnightPromotionCapture:
                promotionBoard = (turn == White) ? &whiteKnights : &blackKnights;
                break;
            case Move::BishopPromotion: case Move::BishopPromotionCapture:
                promotionBoard = (turn == White) ? &whiteBishops : &blackBishops;
                break;
            case Move::RookPromotion: case Move::RookPromotionCapture:
                promotionBoard = (turn == White) ? &whiteRooks : &blackRooks;
                break;
            case Move::QueenPromotion: case Move::QueenPromotionCapture:
                promotionBoard = (turn == White) ? &whiteQueen : &blackQueen;
                break;
        }

        CLEAR_BIT(*movingBoard, toSquare); // Remove temporary pawn move
        SET_BIT(*promotionBoard, toSquare); // Set new piece type as present
    }

    // Update boards for cumulative bitboards
    if (turn == White) {
        CLEAR_BIT(whitePieces, fromSquare);
        SET_BIT(whitePieces, toSquare);
        if (move.isCapture())
            CLEAR_BIT(blackPieces, toSquare);
    } else {
        CLEAR_BIT(blackPieces, fromSquare);
        SET_BIT(blackPieces, toSquare);
        if (move.isCapture())
            CLEAR_BIT(whitePieces, toSquare);
    }
}

// Take back the last move made
void Chessboard::pop() {
    Move lastMove = pastMoves.back();
    pastMoves.pop_back();
    Square fromSquare = lastMove.getFromSquare(), toSquare = lastMove.getToSquare();
    PieceType fromPiece = pieceAt(fromSquare);

    // Move the piece back to its original position
    // To and from squares are inversed from the original order to accomplish this
    Move undoMove = Move(toSquare, fromSquare, Move::Quiet);
    this->push(undoMove);
    pastMoves.pop_back(); // Pop a second time to remove the undo move from history

    // Undo king movement castling right loss
    // YOU CANT JUST SET COLORS RIGHTS TO TRUE, HAVE TO CHECK FOR LOSS FROM ROOKS SOMEHOW
    if (fromPiece == PieceType::King) {
        if (turn == Black && fromSquare == Square::e1) { // Reverse turn condition as it is in reference to the last turn
            whiteKingCastle = whiteQueenCastle = true;
        } else if (turn == White && fromSquare == Square::e8) {
            blackKingCastle = blackQueenCastle = true;
        }
    }

    // Undo rook movement castling right loss

    if (fromSquare == Square::h1)
        whiteKingCastle = true;
    if (fromSquare == Square::a1)
        whiteQueenCastle = true;
    if (fromSquare == Square::h8)
        blackKingCastle = true;
    if (fromSquare == Square::a8)
        blackQueenCastle = true;

    // Handle en passant

    // Handle promotions
}