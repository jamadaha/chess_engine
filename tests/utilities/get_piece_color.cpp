#include <stdexcept>
#include <string>
#include <stdlib.h>

#include "../../classes/headers/constants.hh"
#include "../../classes/headers/utilities.hh"

int main(int argc, char* argv[]) {
    Color expectedColor = (Color) std::atoi(argv[1]);
    PieceChar piece = (PieceChar) argv[2][0];

    Color actualColor = (Color) Utilities::GetPieceColor(piece);

    if (expectedColor == actualColor)
        exit(EXIT_SUCCESS);
    else
        throw std::logic_error("Unexpected Color | Expected " + std::to_string((int)expectedColor) + " - Actual " + std::to_string((int)actualColor));
}