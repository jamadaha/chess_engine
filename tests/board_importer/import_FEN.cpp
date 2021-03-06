#include <stdexcept>
#include <string>
#include <stdlib.h>

#include <string>

#include "../../classes/headers/board_importer.hh"
#include "../../classes/headers/constants.hh"
#include "../../classes/headers/bit_board.hh"

int main(int argc, char* argv[]) {
    std::string FEN = std::string(argv[1]);
    BitBoard expectedBoard = BitBoard();
    expectedBoard.Initialize();

    BitBoard* boardFEN = new BitBoard();

    BoardImporter::ImportFEN(boardFEN, FEN);

    for (int x = 0; x < WIDTH; x++) 
        for (int y = 0; y < HEIGHT; y++)
            if (expectedBoard.GetPiece(Utilities::GetSquare(x, y)) != 
                boardFEN->GetPiece(Utilities::GetSquare(x, y)))
                throw std::logic_error("Unexpected Piece | Expected " 
                                + std::to_string((char)expectedBoard.GetPiece(Utilities::GetSquare(x, y))) + 
                " - Actual "    + std::to_string((char)boardFEN->GetPiece(Utilities::GetSquare(x, y))) + 
                + " | Expected "+ ((char)expectedBoard.GetPiece(Utilities::GetSquare(x, y))) + 
                " - Actual "    + ((char)boardFEN->GetPiece(Utilities::GetSquare(x, y))) + 
                " | At pos "    + std::to_string(x) + "," + std::to_string(y));
            


    exit(EXIT_SUCCESS);  
}