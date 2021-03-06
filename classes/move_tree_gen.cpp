#include "headers/move_tree_gen.hh"

MoveTreeGenerator::MoveTreeGenerator(BitBoard* board, std::string outputPath) : board(board), outputPath(outputPath) {
    evaluators[(int) Color::White] = new Evaluator(Color::White);
    evaluators[(int) Color::Black] = new Evaluator(Color::Black);
    moveGens[(int) Color::White] = new MoveGen(Color::White);
    moveGens[(int) Color::Black] = new MoveGen(Color::Black);
    fp = fopen64(outputPath.c_str(), "w");   
}

MoveTreeGenerator::~MoveTreeGenerator() {
    fclose(fp);
}

void MoveTreeGenerator::GenerateTreeToFile(int depth, int outputDepth) {
    MoveTreeNode moveTree = GenerateMoveTree(depth, outputDepth);
    PrintNode("", moveTree, 0, true);
}

void MoveTreeGenerator::PrintNode(std::string move, MoveTreeNode node, int indent, bool lastNode) {
    int newIndent = indent + 2;
    PrintIndentNL(indent, "{");
    PrintIndentNL(newIndent, "\"move\": \"" + move + "\",");
    PrintIndentNL(newIndent, "\"score\": \"" + std::to_string(node.score) + "\",");
    PrintIndentNL(newIndent, "\"materialScore\": \"" + std::to_string(node.materialScore) + "\",");
    PrintIndentNL(newIndent, "\"positionScore\": \"" + std::to_string(node.positionScore) + "\",");

    if (node.children.size() == 0)
        PrintIndentNL(newIndent, "\"moves\": []");
    else {
        PrintIndentNL(newIndent, "\"moves\": [");
        int index = 0;
        for (std::map<std::string, MoveTreeNode>::iterator it = node.children.begin(); it !=node.children.end(); ++it)
            PrintNode(it->first, it->second, newIndent + 2, ++index == node.children.size());
            
        PrintIndentNL(newIndent, "]");
    }
    if (lastNode)
        PrintIndentNL(indent, "}");
    else
        PrintIndentNL(indent, "},");
}

MoveTreeNode MoveTreeGenerator::GenerateMoveTree(int depth, int outputDepth) {
    U64 attacks = 0;
    Move* moves = (Move*) calloc(256, sizeof(Move));
    int moveCount = moveGens[(int) board->GetColor()]->GetAllMoves(moves, *board, &attacks);
    free(moves);
    MoveTreeNode node = NegaMax(depth, depth - outputDepth, attacks);
    return node;
}

MoveTreeNode MoveTreeGenerator::NegaMax(int depth, int outputDepth, U64 attacks) {
    if (depth == 0)
        return MoveTreeNode(evaluators[(int) board->GetColor()]->EvaluatePieceCount(*board), evaluators[(int) board->GetColor()]->EvaluatePositionValue(*board));
    // 218 I believe to be the max number of moves - as such its rounded up to 256
    Move* moves = (Move*) calloc(256, sizeof(Move));
    U64 attackSquares = attacks;
    int moveCount = moveGens[(int) board->GetColor()]->GetAllMoves(moves, *board, &attackSquares);
    
    MoveTreeNode node = MoveTreeNode(-(int)PieceValue::Inf, depth);

    for (int i = 0; i < moveCount; i++) {
        board->DoMove(moves[i]);
        MoveTreeNode childNode = NegaMax(depth - 1, outputDepth, attackSquares);
        childNode.score *= -1;
        childNode.materialScore *= -1;
        childNode.positionScore *= -1;
        board->UndoMove(moves[i]);
        if (depth >= outputDepth)
            node.AppendMoveTreeNode(moves[i].ToString(), childNode);

        if (childNode.score > node.score) {
            node.score = childNode.score;
            node.materialScore = childNode.materialScore;
            node.positionScore = childNode.positionScore;
        }
    }
    free(moves);
    return node;
}

void MoveTreeGenerator::PrintIndent(int indent, std::string string) {
    fprintf(fp, "%*s%s", indent, "", string.c_str());
}

void MoveTreeGenerator::PrintIndentNL(int indent, std::string string) {
    PrintIndent(indent, string);
    fprintf(fp, "\n");
}
