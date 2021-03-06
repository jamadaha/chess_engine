#ifndef MOVE
#define MOVE

#include <string>

#include "constants.hh"
#include "utilities.hh"

class Move {
public:
    MoveType type;
    Square fromSquare;
    Square toSquare;
    Color fromColor;
    Color toColor;
    PieceType fromType;
    PieceType toType;
    Move(MoveType type, Color fromColor) :
    type(type),
    fromColor(fromColor) {}

    Move(MoveType type, Square fromSquare, Square toSquare, Color fromColor, Color toColor, PieceType fromType, PieceType toType) :
    type(type),
    fromSquare(fromSquare),
    toSquare(toSquare),
    fromColor(fromColor),
    toColor(toColor),
    fromType(fromType),
    toType(toType)     {}

    std::string ToString();
private:
};
#endif
