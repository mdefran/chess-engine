#include <stdio.h>
#include <iostream>
#include <cstdint>
#include <list>

enum PieceType {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

enum Color {
    White,
    Black
};

/*
Assignment follows a right-to-left, bottom-to-top pattern in reference to the corresponding positions on a chessboard.
Since H1 = 0, the LSB corresponds to the bottom right of the board.
This varies from traditional little-endian encoding of both the ranks and files of the board to provide visual clarity.
*/
enum Square {
    h1, g1, f1, e1, d1, c1, b1, a1,
    h2, g2, f2, e2, d2, c2, b2, a2,
    h3, g3, f3, e3, d3, c3, b3, a3,
    h4, g4, f4, e4, d4, c4, b4, a4,
    h5, g5, f5, e5, d5, c5, b5, a5,
    h6, g6, f6, e6, d6, c6, b6, a6,
    h7, g7, f7, e7, d7, c7, b7, a7,
    h8, g8, f8, e8, d8, c8, b8, a8
};

enum MoveType {
    Quiet,
    Capture,
    Promotion,
    Castle,
    EnPassant
};

/*
Bitboards are 64-bit variables used to represent the location of pieces on the board.
Each bit corresponds to a square on the chessboard. This starts from the bottom left of the board, traversed in row-major order.
The presence of a piece on a square is indicated by a value of 1, while the absence of that piece is indicated by a 0.
*/
typedef uint64_t Bitboard;

/*
Moves are represented by 16-bit variables.
Bits 0-5 represent the destination square.
Bits 6-11 represent the origin square.

For example:

Square 11 (E2) to square 27 (E4).
0000 001011 011011: Represents E2E4, a common opening.
*/
typedef uint16_t Move;
typedef std::list<Move> MovesList;

// File masks
const Bitboard FILE_A = 0x8080808080808080ULL;
const Bitboard FILE_B = 0x4040404040404040ULL;
const Bitboard FILE_C = 0x2020202020202020ULL;
const Bitboard FILE_D = 0x1010101010101010ULL;
const Bitboard FILE_E = 0x0808080808080808ULL;
const Bitboard FILE_F = 0x0404040404040404ULL;
const Bitboard FILE_G = 0x0202020202020202ULL;
const Bitboard FILE_H = 0x0101010101010101ULL;

// Rank masks
const Bitboard RANK_1 = 0x00000000000000FFULL;
const Bitboard RANK_2 = 0x000000000000FF00ULL;
const Bitboard RANK_3 = 0x0000000000FF0000ULL;
const Bitboard RANK_4 = 0x00000000FF000000ULL;
const Bitboard RANK_5 = 0x000000FF00000000ULL;
const Bitboard RANK_6 = 0x0000FF0000000000ULL;
const Bitboard RANK_7 = 0x00FF000000000000ULL;
const Bitboard RANK_8 = 0xFF00000000000000ULL;

// General masks
const Bitboard EMPTY = 0ULL;
const Bitboard UNIVERSE = 0xFFFFFFFFFFFFFFFFULL;

// Set a bit at a given position to 1
#define SET_BIT(bitboard, bit) ((bitboard) |= (1ULL << bit))
// Set a bit at a given position to 0
#define CLEAR_BIT(bitboard, bit) ((bitboard) &= ~(1ULL << bit))
// Get the value of a bit at a given position
#define GET_BIT(bitboard, bit) ((bitboard) & (1ULL << bit))
// Get the number of trailing zeros after the least significant bit
#define GET_LSB(bitboard) (__builtin_ctzll(bitboard))
// Clear the LSB and return its index
inline int POP_LSB(Bitboard &bitboard) {
    int index = GET_LSB(bitboard);
    /*
    Clears the rightmost bit.
    When you subtract one from a binary number, the rightmost bit is set to 0, and all following bits are set to 1.
    Since the rightmost bit will not be set in bitboard - 1, it will be cleared when &= is applied.
    */
    bitboard &= bitboard - 1;
    return index;
}
#define BITBOARD(square) ((1ULL << square))

// Directional helper functions with boundary validation
constexpr Bitboard north(Bitboard bitboard) { return (bitboard & ~RANK_8) << 8; }
constexpr Bitboard east(Bitboard bitboard) { return (bitboard & ~FILE_H) >> 1; }
constexpr Bitboard south(Bitboard bitboard) { return (bitboard & ~RANK_1) >> 8; }
constexpr Bitboard west(Bitboard bitboard) { return (bitboard & ~FILE_A) << 1; }
constexpr Bitboard northeast(Bitboard bitboard) { return (bitboard & ~RANK_8 & ~FILE_H) << 7; }
constexpr Bitboard northwest(Bitboard bitboard) { return (bitboard & ~RANK_8 & ~FILE_A) << 9; }
constexpr Bitboard southwest(Bitboard bitboard) { return (bitboard & ~RANK_1 & ~FILE_A) >> 7; }
constexpr Bitboard southeast(Bitboard bitboard) { return (bitboard & ~RANK_1 & ~FILE_H) >> 9; }

void printBitboard(Bitboard bitboard) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 7; file >= 0; file--) {
            int square = rank * 8 + file;
            Bitboard mask = 1ULL << square;
            if (bitboard & mask)
                std::cout << "1 ";
            else
                std::cout << "0 ";
        }
        std::cout << std::endl;
    }
}

struct Chessboard {
    // Member variables
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

    Color turn;
    
    // Declare lookup tables for leaping pieces
    Bitboard knightAttacks[64], kingAttacks[64], whitePawnAttacks[64], blackPawnAttacks[64];

    // Move lists for move generation
    MovesList pseudoLegalMoves, legalMoves;

    // Variable to store double pawn moves that could enable an en passant
    short enPassant;
    bool whiteQueenCastle, whiteKingCastle, blackQueenCastle, blackKingCastle;

    Chessboard() {
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
        blackPieces = 0xFFFF00000F0000ULL;

        turn = White;
        
        // Generate attack map lookup tables for knights and kings
        for (int square = 0; square < 64; square++) {
            Bitboard fromSquare = BITBOARD(square);

            kingAttacks[square] = north(fromSquare) | northeast(fromSquare) | east(fromSquare) | 
                                  southeast(fromSquare) | south(fromSquare) | southwest(fromSquare) |
                                  west(fromSquare) | northwest(fromSquare);

            // Edit boundary checking to within fromSquare to bitboard conversion if causing errors
            knightAttacks[square] = (((fromSquare << 6) | (fromSquare >> 10)) & ~(FILE_A | FILE_B)) |
                                    (((fromSquare << 15) | (fromSquare >> 17)) & ~(FILE_A)) |
                                    (((fromSquare << 17) | (fromSquare >> 15)) & ~(FILE_H)) |
                                    (((fromSquare << 10) | (fromSquare >> 6)) & ~(FILE_G | FILE_H));

            //whitePawnAttacks[square] = ((fromSquare & RANK_2) != 0) ? north(fromSquare) | (fromSquare << 16) : (north(fromSquare) & ~(RANK_8));
            //blackPawnAttacks[square] = ((fromSquare & RANK_7) != 0) ? south(fromSquare) | (fromSquare >> 16) : (south(fromSquare) & ~(RANK_1));
            whitePawnAttacks[square] = north(fromSquare);
            blackPawnAttacks[square] = south(fromSquare);
        }
    }

    // Determine if the opponent has a piece on a given square and return true if so
    bool enemyAt(short square) {
        Bitboard enemyPieces = (turn == White) ? blackPieces : whitePieces;
        return enemyPieces & BITBOARD(square);
    }

    // Efficiently store a move in the pseudo legal moves list
    void addMove(short fromSquare, short toSquare) {
        Move move = fromSquare | (toSquare << 6);
        pseudoLegalMoves.push_back(move);
    }

    // Generate pseudo legal king moves for the current player using the attack maps
    void generateKingMoves() {
        short fromSquare = (turn == White) ? POP_LSB(whiteKing) : POP_LSB(blackKing);
        Bitboard toSquares = kingAttacks[fromSquare];
        while (toSquares != 0) {
            addMove(fromSquare, POP_LSB(toSquares));
        }
    }

    // Generate pseudo legal knight moves for the current player using the attack maps
    void generateKnightMoves() {
        Bitboard fromSquares = (turn == White) ? whiteKnights : blackKnights;
        while (fromSquares != 0) {
            short fromSquare = POP_LSB(fromSquares);
            Bitboard toSquares = knightAttacks[fromSquare];
            while (toSquares != 0) {
                addMove(fromSquare, POP_LSB(toSquares));
            }
        }
    }

    // Generate pseudo legal pawn moves for the current player 
    void generatePawnMoves() {
        Bitboard fromSquares = (turn == White) ? whitePawns : blackPawns;
        if (turn == White) {
            while (fromSquares != 0) {
                Bitboard fromSquare = BITBOARD(POP_LSB(fromSquares));
                Bitboard toSquares = whitePawnAttacks[fromSquare];
                // If there is an enemy piece in capture range, add the capture to the possible moves
                if ((northeast(fromSquare) & blackPieces) != 0)
                    toSquares |= (northeast(fromSquare));
                if ((northwest(fromSquare) & blackPieces) != 0)
                    toSquares |= (northwest(fromSquare));
                // If it is the pawn's first move and there are no pieces in the way, add the double advance to the possible moves
                if ((fromSquare & RANK_2) != 0 && (((fromSquare << 8) | (fromSquare << 16)) & (whitePieces | blackPieces)) == 0) {
                    toSquares |= (fromSquare << 16);
                    // Store the move as a possible en passant
                    enPassant = fromSquare;
                }
            }
        }
        else {
            while (fromSquares != 0) {
                Bitboard fromSquare = BITBOARD(POP_LSB(fromSquares));
                Bitboard toSquares = blackPawnAttacks[fromSquare];
                // If there is an enemy piece in capture range, add the capture to the possible moves
                if ((southeast(fromSquare) & whitePieces) != 0)
                    toSquares |= (southeast(fromSquare));
                if ((southwest(fromSquare) & whitePieces) != 0)
                    toSquares |= (southwest(fromSquare));
                // If it is the pawn's first move and there are no pieces in the way, add the double advance to the possible moves
                if ((fromSquare & RANK_2) != 0 && (((fromSquare >> 8) | (fromSquare >> 16)) & (whitePieces | blackPieces)) == 0) {
                    toSquares |= (fromSquare >> 16);
                    // Store the move as a possible en passant
                    enPassant = fromSquare;
                }
            }
        }
    }

    void printPseudoMoves() {
        for (const auto &move : pseudoLegalMoves) {
            short from = move & 0x3F;
            short to = (move >> 6) & 0x3F;
            std::cout << static_cast<Square>(from) << " " << static_cast<Square>(to) << std::endl;
        }
    }

    void generateLegalMoves() {

    }
};

int main () {
    return 0;
}