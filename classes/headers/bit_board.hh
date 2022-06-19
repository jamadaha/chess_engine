#ifndef BITBOARD
#define BITBOARD

#include <string>

#include "board.hh"
#include "constants.hh"
#include "utilities.hh"

class BitBoard : public Board {
public:
    BitBoard();
    void ClearBoard();
    void Initialize();
    PieceChar GetPiece(Square square);
    void MovePiece(const int startX, const int startY, const int endX, const int endY);
    void PlacePiece(Square square, PieceChar pieceChar);

private:
    Color color;
    U64 pieceBB[PIECECOUNT];
    U64 colorBB[COLORCOUNT];
    U64 occupiedBB;
    U64 emptyBB;

    void PlacePiece(Square square, PieceType type, Color color);
    
    friend class BitEvaluator;
};
#endif