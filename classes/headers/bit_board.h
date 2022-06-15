#ifndef BITBOARD
#define BITBOARD

#define U64 unsigned long long int
#define C64(x) (U64)1 << (U64) x

#include <string>

#include "board.h"
#include "constants.h"
#include "utilities.h"

class BitBoard : public Board {
public:
    BitBoard();
    void ClearBoard();
    void Initialize();
    PieceChar GetPiece(Square square);
    void MovePiece(const int startX, const int startY, const int endX, const int endY);
    void PlacePiece(Square square, PieceChar pieceChar);

private:
    U64 pieceBB[PIECECOUNT];
    U64 colorBB[COLORCOUNT];
    U64 occupiedBB;
    U64 emptyBB;

    void PlacePiece(Square square, PieceType type, Color color);
    
};
#endif