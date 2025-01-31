#include "movegen.h"
#include <intrin.h>
#include "data.h"

Bitboard allMovesForPiece(Bitboard board, MoveLookup &lookup, int playerIndex, int pieceIndex)
{
    return lookup[playerIndex][pieceIndex] & ~board;
}

Bitboard allMovesForPiece(Bitboard piece, Bitboard board, MoveLookup &lookup, int playerIndex)
{
    int index = 31 - _bit_scan_forward(piece);
    return allMovesForPiece(board, lookup, playerIndex, index);
}

Bitboard allMovesForBoard(Bitboard board, MoveLookup *lookups, int playerIndex)
{
    Bitboard boardPiecesToGo = board & boardMask;
    Bitboard moves = 0;
    Bitboard lastCard = 0;

    for (int cardNum = 0; cardNum < 2; ++cardNum) {
        int cardBitPosition = _bit_scan_forward(board ^ lastCard);
        while (boardPiecesToGo) {
            Bitboard boardSquare = boardPiecesToGo & -boardPiecesToGo;
            boardPiecesToGo ^= boardSquare;
            // We're rewriting the same code as allMovesForPiece instead of using the function since
            // we only want to do ~board at the very end instead of every time we do a lookup.
            // This is purely for performance reasons.
            // TODO: Check if this actually makes a difference.
            int index = 31 - _bit_scan_forward(boardSquare);
            moves |= lookups[cardBitPosition][playerIndex][index];
        }
    }
    moves &= ~board;

    return moves;
}

int nextStatesForBoard(State state, MoveLookup *lookups, int playerIndex, StateArray &nextStates)
{
    int nextStateSize = 0;
    Bitboard boardAndCards = state.allPieces[playerIndex];
    Bitboard enemyPiecesAndCards = state.allPieces[1 - playerIndex];
    Bitboard boardOnly = boardAndCards & boardMask;

    // Get all the next states for the entire board for both cards
    Bitboard boardPiecesToGo = boardOnly;
    while (boardPiecesToGo) {
        Bitboard pieceSquare = boardPiecesToGo & -boardPiecesToGo;
        boardPiecesToGo ^= pieceSquare;

        // Get all the next states for this piece for both cards
        Bitboard lastCard = 0;
        for (int cardNum = 0; cardNum < 2; ++cardNum) {
            int cardBitPosition = _bit_scan_forward(boardAndCards ^ lastCard);

            Bitboard cardBit = 1 << cardBitPosition;
            lastCard = cardBit;

            Bitboard moveMask = allMovesForPiece(pieceSquare, boardOnly,
                                                 lookups[cardBitPosition], playerIndex);
            Bitboard newCards = boardAndCards & ~boardMask ^ cardBit | (state.kings & ~boardMask);
            bool kingIsMoving = pieceSquare & state.kings;

            // Get all the next states for this piece for the current card
            Bitboard moveMaskPiecesToGo = moveMask;
            while (moveMaskPiecesToGo) {
                Bitboard moveSquare = moveMaskPiecesToGo & -moveMaskPiecesToGo;
                moveMaskPiecesToGo ^= moveSquare;

                Bitboard newPieces = boardOnly ^ pieceSquare | moveSquare | newCards;
                nextStates[nextStateSize].allPieces[playerIndex] = newPieces;
                nextStates[nextStateSize].allPieces[1 - playerIndex] = enemyPiecesAndCards & ~moveSquare;
                Bitboard newKings = state.kings & boardMask & ~moveSquare | cardBit;
                if (kingIsMoving)
                    newKings = newKings & ~pieceSquare | moveSquare;
                nextStates[nextStateSize].kings = newKings;
                nextStateSize += 1;
            }
        }
    }

    // If the number of next states is still 0, it means that there are no legal moves for either card
    // The rule in this situation is to swap a card in the hand for the card on the side
    if (nextStateSize == 0) {
        Bitboard kingBit = _bit_scan_forward(state.kings);

        int card0BitPosition = _bit_scan_forward(boardAndCards);
        Bitboard card0Bit = 1 << card0BitPosition;
        Bitboard boardNoCard0 = boardAndCards ^card0Bit;

        int card1BitPosition = _bit_scan_forward(boardNoCard0);
        Bitboard card1Bit = 1 << card1BitPosition;
        Bitboard boardNoCard1 = boardAndCards ^card1Bit;

        nextStates[0].allPieces[playerIndex] = boardNoCard0 | kingBit;
        nextStates[0].allPieces[1 - playerIndex] = enemyPiecesAndCards;
        nextStates[0].kings = state.kings ^ kingBit | card0Bit;

        nextStates[1].allPieces[playerIndex] = boardNoCard1 | kingBit;
        nextStates[1].allPieces[1 - playerIndex] = enemyPiecesAndCards;
        nextStates[1].kings = state.kings ^ kingBit | card1Bit;

        nextStateSize = 2;
    }

    return nextStateSize;
}

// 0 = p0 win, 1 = p1 win, -1 = game has not ended
int checkWinCondition(State state)
{
    Bitboard kings = state.kings & boardMask;
    if (_mm_popcnt_u32(kings) != 2)
        return _mm_popcnt_u32(kings & state.allPieces[1]);
    if (state.allPieces[0] & state.kings & redStartingSquare)
        return 0;
    if (state.allPieces[1] & state.kings & blueStartingSquare)
        return 1;
    return -1;
}
