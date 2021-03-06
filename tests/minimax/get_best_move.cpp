#include <stdexcept>
#include <string>
#include <stdlib.h>

#include <string>

#include "../../classes/headers/constants.hh"
#include "../../classes/headers/bit_board.hh"
#include "../../classes/headers/board_importer.hh"
#include "../../classes/headers/move.hh"
#include "../../classes/headers/minimax.hh"

int main(int argc, char* argv[]) {
    std::string moveSequence = std::string(argv[1]);
    BitBoard board = BitBoard();
    BoardImporter::ImportMoveSequence(&board, moveSequence);
    std::string expectedMove = argv[3];    

    MiniMax minimax = MiniMax(&board);
    Move move = minimax.GetBestMove(std::atoi(argv[2]));


    if (expectedMove == move.ToString())
        exit(EXIT_SUCCESS);
    else
        throw std::logic_error("Incorrect Hash | Expected " + expectedMove + " - Actual " + move.ToString());
}