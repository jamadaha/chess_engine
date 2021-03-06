#include "headers/move_gen.hh"

MoveGen::MoveGen(Color color):
color(color),
oppColor(Utilities::GetOppositeColor(color)),
up((color == Color::White) ? Direction::North : Direction::South),
upRight((up == Direction::North) ? Direction::NorthEast : Direction::SouthEast),
upLeft((up == Direction::North) ? Direction::NorthWest : Direction::SouthWest),
doubleRank((up == Direction::North) ? Row::Row2 : Row::Row7),
enPassantRank((up == Direction::North) ? Row::Row6 : Row::Row3),
notPromotionRank((up == Direction::North) ? NotEdge::North : NotEdge::South) {
    if (up == Direction::North) {
        castlingBlock[(int) Castling::King] = CastlingBlockSquares::KSideWhite;
        castlingBlock[(int) Castling::Queen] = CastlingBlockSquares::QSideWhite;
        castlingAttack[(int) Castling::King] = CastlingAttackSquares::KSideWhite;
        castlingAttack[(int) Castling::Queen] = CastlingAttackSquares::QSideWhite;
    } else {
        castlingBlock[(int) Castling::King] = CastlingBlockSquares::KSideBlack;
        castlingBlock[(int) Castling::Queen] = CastlingBlockSquares::QSideBlack;
        castlingAttack[(int) Castling::King] = CastlingAttackSquares::KSideBlack;
        castlingAttack[(int) Castling::Queen] = CastlingAttackSquares::QSideBlack;
    }
    GeneratePawnMoves();
    GenerateKnightMoves();
    GenerateKingMoves();
}

int MoveGen::GetAllMoves(Move* moves, BitBoard board, U64* attackedSquares) {
    U64 priorSquares = *attackedSquares;
    *attackedSquares = 0;
    int moveCount = 0;
    bool isKingSafe = IsKingSafe(board);
    moveCount += GetPawnMoves(moves, moveCount, board, isKingSafe, attackedSquares);
    moveCount += GetRookMoves(moves, moveCount, board, isKingSafe, attackedSquares);
    moveCount += GetBishopMoves(moves, moveCount, board, isKingSafe, attackedSquares);
    moveCount += GetQueenMoves(moves, moveCount, board, isKingSafe, attackedSquares);
    moveCount += GetKnightMoves(moves, moveCount, board, isKingSafe, attackedSquares);
    moveCount += GetKingMoves(moves, moveCount, board, isKingSafe, attackedSquares, priorSquares);
    return moveCount;
}

int MoveGen::GetPawnMoves(Move* moves, int startIndex, BitBoard board, bool isKingSafe, U64* attackedSquares) {
    // Generate single and double step moves
    U64 pieces = board.pieceBB[(int) PieceType::Pawn] & board.colorBB[(int) color];
    if (pieces == 0)
        return 0;
        
    int moveCount = 0;

    // Generate attackedSquares
    U64 tempPieces = pieces;
    while (tempPieces) {
        (*attackedSquares) |= pawnCaptureMoves[Utilities::LSB_Pop(&tempPieces)];
    }

    while (pieces) {
        int lsb = Utilities::LSB_Pop(&pieces);

        // Quiet move
        //// Single push
        if (!(board.occupiedBB & pawnSingleMove[(int) lsb]))
            if (isKingSafe || IsKingSafe(board, (board.occupiedBB ^ C64(lsb)) | C64(lsb + (int) up)))
                AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::Quiet, (Square) lsb, (Square) (lsb + (int) up), color, Color::None, PieceType::Pawn, PieceType::None));
        //// Double push
        if (C64(lsb) & (U64) doubleRank && !(board.occupiedBB & pawnDoubleMove[(int) lsb]))
            if (isKingSafe || IsKingSafe(board, (board.occupiedBB ^ C64(lsb)) | C64(lsb + (int) up * 2)))
                AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::Quiet, (Square) lsb, (Square) (lsb + (int) up * 2), color, Color::None, PieceType::Pawn, PieceType::None));
        
        // Attack moves
        //// Diagonal
        U64 captures = board.colorBB[(int) oppColor] & pawnCaptureMoves[(int) lsb];
        while (captures) {
            int capturePiece = Utilities::LSB_Pop(&captures);
            if (isKingSafe || IsKingSafe(board, board.occupiedBB ^ C64(lsb), board.colorBB[(int) oppColor] ^ C64(capturePiece)))
                AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::Capture, (Square) lsb, (Square) capturePiece, color, oppColor, PieceType::Pawn, board.GetType((Square) capturePiece, oppColor)));
        }
        //// En Passant
        captures = board.enPassant & pawnCaptureMoves[(int) lsb] & (U64) enPassantRank;
        while (captures) {
            int capturePiece = Utilities::LSB_Pop(&captures);
            if (isKingSafe || IsKingSafe(board, board.occupiedBB ^ C64(lsb),  board.colorBB[(int) oppColor] ^ C64(capturePiece)))
                AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::EPCapture, (Square) lsb, (Square) capturePiece, color, oppColor, PieceType::Pawn, board.GetType((Square) BitShifts::Shift(capturePiece, up, -1), oppColor)));
        }
    }
    return moveCount;
}

int MoveGen::GetRookMoves(Move* moves, int startIndex, BitBoard board, bool isKingSafe, U64* attackedSquares) {
    Direction directions[4] = { Direction::North, Direction::East, Direction::South, Direction::West };
    int moveCount = 0;

    U64 pieces = board.pieceBB[(int) PieceType::Rook] & board.colorBB[(int) color];
    if (pieces == 0)
        return 0;
    for (int i = 0; i < 4; i++)
        moveCount += GetMoves(moves, startIndex + moveCount, board, pieces, directions[i], PieceType::Rook, isKingSafe, attackedSquares);
    return moveCount;
}

int MoveGen::GetBishopMoves(Move* moves, int startIndex, BitBoard board, bool isKingSafe, U64* attackedSquares) {
    Direction directions[4] = { Direction::NorthEast, Direction::NorthWest, Direction::SouthEast, Direction::SouthWest };
    int moveCount = 0;

    U64 pieces = board.pieceBB[(int) PieceType::Bishop] & board.colorBB[(int) color];
    if (pieces == 0)
        return 0;
    for (int i = 0; i < 4; i++)
        moveCount += GetMoves(moves, startIndex + moveCount, board, pieces, directions[i], PieceType::Bishop, isKingSafe, attackedSquares);
    return moveCount;
}

int MoveGen::GetQueenMoves(Move* moves, int startIndex, BitBoard board, bool isKingSafe, U64* attackedSquares) {
    Direction directions[8] = { Direction::North, Direction::East, Direction::South, Direction::West, 
                                Direction::NorthEast, Direction::NorthWest, Direction::SouthEast, Direction::SouthWest };
    int moveCount = 0;

    U64 pieces = board.pieceBB[(int) PieceType::Queen] & board.colorBB[(int) color];
    if (pieces == 0)
        return 0;
    for (int i = 0; i < 8; i++)
        moveCount += GetMoves(moves, startIndex + moveCount, board, pieces, directions[i], PieceType::Queen, isKingSafe, attackedSquares);
    return moveCount;
}

int MoveGen::GetKnightMoves(Move* moves, int startIndex, BitBoard board, bool isKingSafe, U64* attackedSquares) {
    U64 pieces = board.pieceBB[(int) PieceType::Knight] & board.colorBB[(int) color];
    if (pieces == 0)
        return 0;
    int moveCount = 0;

    while (pieces) {
            int lsbPiece = Utilities::LSB_Pop(&pieces);
            U64 to = knightMoves[lsbPiece];
            (*attackedSquares) |= to;
            // Attack moves
            U64 attackMoves = to & board.colorBB[(int) oppColor];
            to &= ~board.occupiedBB;

            while (attackMoves) {
                int lsb = Utilities::LSB_Pop(&attackMoves);
                if (isKingSafe || IsKingSafe(board, board.occupiedBB ^ C64(lsbPiece), board.colorBB[(int) oppColor] ^ C64(lsb)))
                    AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::Capture, (Square) lsbPiece, (Square) lsb, color, oppColor, PieceType::Knight, board.GetType((Square) lsb, oppColor)));
            }

            // Quiet moves
            U64 quietMoves = to & (~board.occupiedBB);
            while (quietMoves) {
                int lsb = Utilities::LSB_Pop(&quietMoves);
                if (isKingSafe || IsKingSafe(board, (board.occupiedBB ^ C64(lsbPiece) | C64(lsb))))
                    AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::Quiet, (Square) lsbPiece, (Square) lsb, color, Color::None, PieceType::Knight, PieceType::None));
            }
    }

    return moveCount;
}

int MoveGen::GetKingMoves(Move* moves, int startIndex, BitBoard board, bool isKingSafe, U64* attackedSquares, U64 priorAttacks) {
    U64 pieces = board.pieceBB[(int) PieceType::King] & board.colorBB[(int) color];
    if (pieces == 0)
        return 0;
    int moveCount = 0;

    int lsbPiece = Utilities::LSB_Pop(&pieces);
    U64 to = kingMoves[lsbPiece];
    (*attackedSquares) |= to;
    // Attack moves
    U64 attackMoves = to & board.colorBB[(int) oppColor];
    to &= ~board.occupiedBB;
    to &= ~priorAttacks;
    attackMoves &= ~priorAttacks;

    while (attackMoves) {
        int lsb = Utilities::LSB_Pop(&attackMoves);
        if (isKingSafe || IsKingSafe(board, board.occupiedBB ^ C64(lsbPiece), board.colorBB[(int) oppColor] ^ C64(lsb), C64(lsb)))
            AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::Capture, (Square) lsbPiece, (Square) lsb, color, oppColor, PieceType::King, board.GetType((Square) lsb, oppColor)));
    }

    // Quiet moves
    U64 quietMoves = to & (~board.occupiedBB);
    while (quietMoves) {
        int lsb = Utilities::LSB_Pop(&quietMoves);
        if (isKingSafe || IsKingSafe(board, (board.occupiedBB ^ C64(lsbPiece) | C64(lsb)), board.colorBB[(int) oppColor], C64(lsb)))
            AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::Quiet, (Square) lsbPiece, (Square) lsb, color, Color::None, PieceType::King, PieceType::None));
    }

    // Castling
    if (board.castlingAllowed[(int)color][(int) Castling::King] && !(board.occupiedBB & (U64) castlingBlock[(int) Castling::King]) && !(priorAttacks & (U64) castlingAttack[(int) Castling::King]))
        if (isKingSafe)
            AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::KingCastle, color));
    if (board.castlingAllowed[(int)color][(int) Castling::Queen] && !(board.occupiedBB & (U64) castlingBlock[(int) Castling::Queen]) && !(priorAttacks & (U64) castlingAttack[(int) Castling::Queen]))
        if (isKingSafe)
            AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::QueenCastle, color));
        
    return moveCount;
}

int MoveGen::GetMoves(Move* moves, int startIndex, BitBoard board, U64 pieces, Direction direction, PieceType type, bool isKingSafe, U64* attackedSquares) {
    U64 to = pieces;
    int moveCount = 0;
    int counter = 1;
    while (to) {
        to = BitShifts::Shift(to & Utilities::NotEdge(direction), direction, 1) & ~board.colorBB[(int) color];
        (*attackedSquares) |= to;
        U64 attackMoves = to & board.occupiedBB;
        to &= ~board.occupiedBB;

        while (attackMoves) {
            int lsb = Utilities::LSB_Pop(&attackMoves);
            if (isKingSafe || IsKingSafe(board, board.occupiedBB ^ C64(lsb - (int) direction * counter), board.colorBB[(int) oppColor] ^ C64(lsb)))
                AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::Capture, (Square) (lsb - (int) direction * counter), (Square) lsb, color, oppColor, type, board.GetType((Square) lsb, oppColor)));
        }

        U64 quietMoves = to;

        while (quietMoves) {
            int lsb = Utilities::LSB_Pop(&quietMoves);
            if (isKingSafe || IsKingSafe(board, (board.occupiedBB ^ C64(lsb - (int) direction * counter) | C64(lsb))))
                AppendMove(moves, startIndex + moveCount, &moveCount, Move(MoveType::Quiet, (Square) (lsb - (int) direction * counter), (Square) lsb, color, Color::None, type, PieceType::None));
        }

        counter++;
    }

    return moveCount;
}

void MoveGen::GeneratePawnMoves() {
    for (int i = 0; i < 64; i++) {
        U64 bit = C64(i);
        pawnCaptureMoves[i] = 0;
        if (bit & (U64) notPromotionRank) {
            if (bit & (U64) NotEdge::West)
                pawnCaptureMoves[i] |= BitShifts::Shift(bit, upLeft, 1);
            if (bit & (U64) NotEdge::East)    
                pawnCaptureMoves[i] |= BitShifts::Shift(bit, upRight, 1);
            U64 upOne = BitShifts::Shift(bit, up, 1);
            pawnSingleMove[i] |= BitShifts::Shift(bit, up, 1);
            if (bit & (U64) doubleRank) {
                pawnDoubleMove[i] |= upOne;
                pawnDoubleMove[i] |= BitShifts::Shift(bit, up, 2);
            }
        }   
    }
}

void MoveGen::GenerateKnightMoves() {
    for (int i = 0; i < 64; i++) {
        U64 bit = C64(i);
        if (bit & (U64) NotEdgeKnight::North) {
            if (bit & (U64) NotEdge::East) {
                knightMoves[i] |= BitShifts::Shift(BitShifts::Shift(bit, Direction::North, 1), Direction::NorthEast, 1);
            }
            if (bit & (U64) NotEdge::West) {
                knightMoves[i] |= BitShifts::Shift(BitShifts::Shift(bit, Direction::North, 1), Direction::NorthWest, 1);
            }
        }
        if (bit & (U64) NotEdgeKnight::South) {
            if (bit & (U64) NotEdge::East) {
                knightMoves[i] |= BitShifts::Shift(BitShifts::Shift(bit, Direction::South, 1), Direction::SouthEast, 1);
            }
            if (bit & (U64) NotEdge::West) {
                knightMoves[i] |= BitShifts::Shift(BitShifts::Shift(bit, Direction::South, 1), Direction::SouthWest, 1);
            }
        }
        if (bit & (U64) NotEdgeKnight::East) {
            if (bit & (U64) NotEdge::North) {
                knightMoves[i] |= BitShifts::Shift(BitShifts::Shift(bit, Direction::North, 1), Direction::East, 2);
            }
            if (bit & (U64) NotEdge::South) {
                knightMoves[i] |= BitShifts::Shift(BitShifts::Shift(bit, Direction::South, 1), Direction::East, 2);
            }
        }
        if (bit & (U64) NotEdgeKnight::West) {
            if (bit & (U64) NotEdge::North) {
                knightMoves[i] |= BitShifts::Shift(BitShifts::Shift(bit, Direction::North, 1), Direction::West, 2);
            }
            if (bit & (U64) NotEdge::South) {
                knightMoves[i] |= BitShifts::Shift(BitShifts::Shift(bit, Direction::South, 1), Direction::West, 2);
            }
        }
    }
}

void MoveGen::GenerateKingMoves() {
    for (int i = 0; i < 64; i++) {
        U64 bit = C64(i);
        if (bit & (U64) NotEdge::North) {
            kingMoves[i] |= BitShifts::Shift(bit, Direction::North, 1);
            if (bit & (U64) NotEdge::East)
                kingMoves[i] |= BitShifts::Shift(bit, Direction::NorthEast, 1);
            if (bit & (U64) NotEdge::West)
                kingMoves[i] |= BitShifts::Shift(bit, Direction::NorthWest, 1);
        }
        if (bit & (U64) NotEdge::South) {
            kingMoves[i] |= BitShifts::Shift(bit, Direction::South, 1);
            if (bit & (U64) NotEdge::East)
                kingMoves[i] |= BitShifts::Shift(bit, Direction::SouthEast, 1);
            if (bit & (U64) NotEdge::West)
                kingMoves[i] |= BitShifts::Shift(bit, Direction::SouthWest, 1);
        }
        if (bit & (U64) NotEdge::East) {
            kingMoves[i] |= BitShifts::Shift(bit, Direction::East, 1);
        }
        if (bit & (U64) NotEdge::West) {
            kingMoves[i] |= BitShifts::Shift(bit, Direction::West, 1);
        }
    }
}

bool MoveGen::IsKingSafe(BitBoard board, U64 tempOccuracyBoard, U64 tempEnemyBoard, U64 tempKingBoard) {
    U64 kingPos = tempKingBoard;
    int kingPosIndex = Utilities::LSB_Pop(&kingPos);

    U64 enemyRooks = (board.pieceBB[(int) PieceType::Rook] | board.pieceBB[(int) PieceType::Queen]) & tempEnemyBoard;
    U64 enemyBishops = (board.pieceBB[(int) PieceType::Bishop] | board.pieceBB[(int) PieceType::Queen]) & tempEnemyBoard;
    U64 enemyKnights = board.pieceBB[(int) PieceType::Knight] & tempEnemyBoard;
    U64 enemyPawns = board.pieceBB[(int) PieceType::Pawn] & tempEnemyBoard;

    if (BitShifts::rays[kingPosIndex][(int) DirectionIndex::North] & enemyRooks)
        if (Utilities::LSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::North] & enemyRooks) == Utilities::LSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::North] & tempOccuracyBoard))
            return false;
    if (BitShifts::rays[kingPosIndex][(int) DirectionIndex::East] & enemyRooks)
        if (Utilities::LSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::East] & enemyRooks) == Utilities::LSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::East] & tempOccuracyBoard))
            return false;
    if (BitShifts::rays[kingPosIndex][(int) DirectionIndex::South] & enemyRooks)
        if (Utilities::MSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::South] & enemyRooks) == Utilities::MSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::South] & tempOccuracyBoard))
            return false;
    if (BitShifts::rays[kingPosIndex][(int) DirectionIndex::West] & enemyRooks)
        if (Utilities::MSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::West] & enemyRooks) == Utilities::MSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::West] & tempOccuracyBoard))
            return false;

    if (BitShifts::rays[kingPosIndex][(int) DirectionIndex::NorthEast] & enemyBishops)
        if (Utilities::LSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::NorthEast] & enemyBishops) == Utilities::LSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::NorthEast] & tempOccuracyBoard))
            return false;
    if (BitShifts::rays[kingPosIndex][(int) DirectionIndex::NorthWest] & enemyBishops)
        if (Utilities::LSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::NorthWest] & enemyBishops) == Utilities::LSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::NorthWest] & tempOccuracyBoard))
            return false;
    if (BitShifts::rays[kingPosIndex][(int) DirectionIndex::SouthEast] & enemyBishops)
        if (Utilities::MSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::SouthEast] & enemyBishops) == Utilities::MSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::SouthEast] & tempOccuracyBoard))
            return false;
    if (BitShifts::rays[kingPosIndex][(int) DirectionIndex::SouthWest] & enemyBishops)
        if (Utilities::MSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::SouthWest] & enemyBishops) == Utilities::MSB(BitShifts::rays[kingPosIndex][(int) DirectionIndex::SouthWest] & tempOccuracyBoard))
            return false;

    if (knightMoves[kingPosIndex] & enemyKnights)
        return false;

    if (pawnCaptureMoves[kingPosIndex] & enemyPawns)
        return false;

    return true;
}

bool MoveGen::IsKingSafe(BitBoard board, U64 tempOccuracyBoard, U64 tempEnemyBoard) {
    return IsKingSafe(board, tempOccuracyBoard, tempEnemyBoard, board.pieceBB[(int) PieceType::King] & board.colorBB[(int) color]);
}

bool MoveGen::IsKingSafe(BitBoard board, U64 tempOccuracyBoard) {
    return IsKingSafe(board, tempOccuracyBoard, board.colorBB[(int) oppColor]);
}

bool MoveGen::IsKingSafe(BitBoard board) {
    return IsKingSafe(board, board.occupiedBB);
}

bool MoveGen::IsSafeMove(BitBoard board, Square square) {

    return true;
}

void MoveGen::AppendMove(Move* moves, int index, int* moveCount, Move move) {
    moves[index] = move;
    (*moveCount)++;
}
