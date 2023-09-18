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
    whiteQueenCastleBeforeMove = whiteKingCastleBeforeMove = blackQueenCastleBeforeMove = blackKingCastleBeforeMove = true;

    // Set attack table values
    this->initializeLookupTables();
}

// Return the type of piece present at a given square
PieceType Chessboard::pieceAt(Square square) {
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

// Pass board control to the opponent
void Chessboard::passTurn() {
    turn = (turn == White) ? Black : White;
}

// Push a move onto the board
void Chessboard::push(Move move) {
    // Push the move to history
    pastMoves.push_back(move);

    // Get move information
    Square fromSquare = move.getFromSquare(), toSquare = move.getToSquare();
    Move::MoveType moveType = move.getMoveType();
    PieceType fromPiece = pieceAt(fromSquare);
    PieceType toPiece = (move.isCapture()) ? pieceAt(toSquare) : PieceType::None;

    // Log castling information before move for future popping
    whiteKingCastleBeforeMove = whiteKingCastle;
    whiteQueenCastleBeforeMove = whiteQueenCastle;
    blackKingCastleBeforeMove = blackKingCastle;
    blackQueenCastleBeforeMove = blackQueenCastle;

    // If a player's king moves from the starting position, disable castling for that player
    if (fromPiece == PieceType::King) {
        if (turn == White && fromSquare == Square::e1)
            whiteKingCastle = whiteQueenCastle = false;
        else if (turn == Black && fromSquare == Square::e8)
            blackKingCastle = blackQueenCastle = false;
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

    // Handle rook movement for castling
    if (turn == White && moveType == Move::KingCastle) {
        whiteKingCastle = whiteQueenCastle = false;
        this->push(Move(Square::h1, Square::f1, Move::Quiet));
        this->pop(); // Remove the extra move just logged as castling is recorded as one move
    } else if (turn == White && moveType == Move::QueenCastle) {
        whiteKingCastle = whiteQueenCastle = false;
        this->push(Move(Square::a1, Square::d1, Move::Quiet));
        this->pop();
    } else if (turn == Black && moveType == Move::KingCastle) {
        blackKingCastle = blackQueenCastle = false;
        this->push(Move(Square::h8, Square::f8, Move::Quiet));
        this->pop();
    } else if (turn == Black && moveType == Move::QueenCastle) {
        blackKingCastle = blackQueenCastle = false;
        this->push(Move(Square::a8, Square::d8, Move::Quiet));
        this->pop();
    }

    // Store en passant squares if necessary
    if (moveType == Move::DoublePawnPush)
        enPassant = move.getToSquare();

    // Perform en passant pawn capture
    if (moveType == Move::EnPassant) {
        if (BITBOARD(toSquare) == northeast(BITBOARD(fromSquare)) || BITBOARD(toSquare) == northwest(BITBOARD(fromSquare))) {
            Square captureSquare = static_cast<Square>(GET_LSB(south(BITBOARD(toSquare))));
            CLEAR_BIT(blackPawns, captureSquare); // We know the captured pawn's color based on the direction it advances in
        } else if (BITBOARD(toSquare) == southeast(BITBOARD(fromSquare)) || BITBOARD(toSquare) == southwest(BITBOARD(fromSquare))) {
            Square captureSquare = static_cast<Square>(GET_LSB(north(BITBOARD(toSquare))));
            CLEAR_BIT(whitePawns, captureSquare);
        }
        capturedPieces.push_back(PieceType::Pawn);
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
                // Keep captured pieces in sync with move history
                capturedPieces.push_back(PieceType::None);
                break;
            case PieceType::Pawn:
                capturedPieces.push_back(PieceType::Pawn);
                captureBoard = (turn == White) ? &blackPawns : &whitePawns;
            case PieceType::Knight:
                capturedPieces.push_back(PieceType::Knight);
                captureBoard = (turn == White) ? &blackKnights : &whiteKnights;
                break;
            case PieceType::Bishop:
                capturedPieces.push_back(PieceType::Bishop);
                captureBoard = (turn == White) ? &blackBishops : &whiteBishops;
                break;
            case PieceType::Rook:
                capturedPieces.push_back(PieceType::Rook);
                captureBoard = (turn == White) ? &blackRooks : &whiteRooks;
                break;
            case PieceType::Queen:
                capturedPieces.push_back(PieceType::Queen);
                captureBoard = (turn == White) ? &blackQueen : &whiteQueen;
                break;
            case PieceType::King:
                capturedPieces.push_back(PieceType::King);
                captureBoard = (turn == White) ? &blackKing : &whiteKing;
                break;
        }

        // Clear the piece and add it to captured piece history
        CLEAR_BIT(*captureBoard, toSquare);
    }

    // Perform promotions
    if (move.isPromotion()) {
        Bitboard *promotionBoard;
        switch (moveType) {
            case Move::KnightPromotion:
            case Move::KnightPromotionCapture:
                promotionBoard = (turn == White) ? &whiteKnights : &blackKnights;
                break;
            case Move::BishopPromotion:
            case Move::BishopPromotionCapture:
                promotionBoard = (turn == White) ? &whiteBishops : &blackBishops;
                break;
            case Move::RookPromotion:
            case Move::RookPromotionCapture:
                promotionBoard = (turn == White) ? &whiteRooks : &blackRooks;
                break;
            case Move::QueenPromotion:
            case Move::QueenPromotionCapture:
                promotionBoard = (turn == White) ? &whiteQueen : &blackQueen;
                break;
        }

        CLEAR_BIT(*movingBoard, toSquare); // Remove temporary pawn move
        SET_BIT(*promotionBoard, toSquare); // Set new piece type as present
    }

    // Update cumulative bitboards
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
    allPieces = whitePieces | blackPieces;

    // Transfer control of the board to the opponent
    this->passTurn();
}

// Take back the last move made
void Chessboard::pop() {
    // Get information about the last move
    Move lastMove = pastMoves.back();
    Square fromSquare = lastMove.getFromSquare(), toSquare = lastMove.getToSquare();
    PieceType fromPiece = pieceAt(fromSquare);
    Move::MoveType moveType = lastMove.getMoveType();

    // Delete the move from history
    pastMoves.pop_back();

    // Transfer control of the board back to the player who made the move being popped
    turn = (turn == White) ? Black : White;

    // Undo promotions
    if (lastMove.isPromotion()) {
        // Determine piece type of the promoted pawn
        PieceType promotedPiece;
        switch (moveType) {
            case Move::KnightPromotion:
            case Move::KnightPromotionCapture:
                promotedPiece = PieceType::Knight;
                break;
            case Move::BishopPromotion:
            case Move::BishopPromotionCapture:
                promotedPiece = PieceType::Bishop;
                break;
            case Move::RookPromotion:
            case Move::RookPromotionCapture:
                promotedPiece = PieceType::Rook;
                break;
            case Move::QueenPromotion:
            case Move::QueenPromotionCapture:
                promotedPiece = PieceType::Queen;
                break;
        }

        // Reset the promoted piece back to a pawn
        Bitboard *promotionBoard;
        switch (promotedPiece) {
            case PieceType::Knight:
                promotionBoard = (turn == White) ? &whiteKnights : &blackKnights;
                break;
            case PieceType::Bishop:
                promotionBoard = (turn == White) ? &whiteBishops : &blackBishops;
                break;
            case PieceType::Rook:
                promotionBoard = (turn == White) ? &whiteRooks : &blackRooks;
                break;
            case PieceType::Queen:
                promotionBoard = (turn == White) ? &whiteQueen : &blackQueen;
                break;
        }

        CLEAR_BIT(*promotionBoard, toSquare);
        SET_BIT(*promotionBoard, fromSquare);
    }

    // Undo castling right loss
    whiteKingCastle = whiteKingCastleBeforeMove;
    whiteQueenCastle = whiteQueenCastleBeforeMove;
    blackKingCastle = blackKingCastleBeforeMove;
    blackQueenCastle = blackQueenCastleBeforeMove;

    // Undo rook movement from castling
    // REVERSE CONDITIONS, THIS IS FROM PUSH
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

    // Replace captured pieces
    if (lastMove.isCapture() && moveType != Move::EnPassant) {
        
    }

    // Handle pawns captured by en passant
    if (moveType == Move::EnPassant) {
        Bitboard *enPassantBoard = (turn == White) ? &whitePawns : &blackPawns; // Opposite of player who made the last move, as we are restoring the piece from 2 moves ago
        SET_BIT(*enPassantBoard, GET_LSB(enPassant));
        capturedPieces.pop_back();
    }

    // Move the piece back to its original position
    Move undoMove = Move(toSquare, fromSquare, Move::Quiet); // To and from squares reversed
    this->push(undoMove);
    pastMoves.pop_back(); // Pop a second time to remove the undo move from history
}