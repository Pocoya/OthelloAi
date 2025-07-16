#pragma once
#include "board.hpp"
#include <array>

// Precomputed masks and patterns
constexpr uint64_t CORNERS       = 0x8100000000000081ULL; // a1, a8, h1, h8
constexpr uint64_t X_SQUARES     = 0x4200000000000042ULL; // b2, b7, g2, g7
constexpr uint64_t C_SQUARES     = 0x2400810000810024ULL; // c3, c6, f3, f6
constexpr uint64_t EDGES         = 0xFF818181818181FFULL; // a2-h2, a7-h7, a2-a7, h2-h7
constexpr uint64_t STABLE_MASK[] = {
    0xFF000000000000FF, // Full edge
    0xE7000000000000E7, // Semi-stable
    0xC3000000000000C3, 
    0x8100000000000081
};


enum GamePhase { EARLY_GAME, MID_GAME, LATE_GAME };
template<GamePhase P> struct Weights;

template<> struct Weights<EARLY_GAME> {
    static constexpr int corner = 15, position = 3, edge = 2, mobility = 1, disc = 0;
};

template<> struct Weights<MID_GAME> {
    static constexpr int corner = 8, position = 2, edge = 2, mobility = 2, disc = 1;
};

template<> struct Weights<LATE_GAME> {
    static constexpr int corner = 3, position = 1, edge = 1, mobility = 0, disc = 3;
};

// Precomputed positional values
int get_positional_score(uint64_t pieces) {
    static constexpr std::array<int, 64> POSITIONAL_TABLE = {
         1000, -300,  100,   80,   80,  100, -300, 1000,
         -300, -500,  -50,  -50,  -50,  -50, -500, -300,
          100,  -50,   30,   20,   20,   30,  -50,  100,
           80,  -50,   20,    5,    5,   20,  -50,   80,
           80,  -50,   20,    5,    5,   20,  -50,   80,
          100,  -50,   30,   20,   20,   30,  -50,  100,
         -300, -500,  -50,  -50,  -50,  -50, -500, -300,
         1000, -300,  100,   80,   80,  100, -300, 1000
    };
    
    int score = 0;
    uint64_t mask = 1;
    for (int i = 0; i < 64; ++i, mask <<= 1) {
        if (pieces & mask) score += POSITIONAL_TABLE[i];
    }
    return score;
}

// Optimized edge stability using precomputed patterns
int edge_stability(uint64_t player) {
    int stable = 0;
    for (const auto& pattern : STABLE_MASK) {
        stable += __builtin_popcountll(player & pattern); // Count stable edges
    }
    return stable;
}

// mobility calculation
template<bool IsBlack>
int calculate_mobility(const Board& board) {
    const uint64_t player = IsBlack ? board.black : board.white;
    const uint64_t opponent = IsBlack ? board.white : board.black;
    const uint64_t empty = ~(player | opponent);
    
    uint64_t mobility = 0;
    for (int dir = 0; dir < 8; ++dir) {
        uint64_t candidates = Board::shift(opponent, dir) & empty;
        while (candidates) {
            uint64_t sq = candidates & -candidates;
            mobility |= sq;
            candidates ^= sq;
        }
    }
    return __builtin_popcountll(mobility); // Number of legal moves
}

template<GamePhase P>
int phase_evaluation(const Board& board) {
    const int positional = get_positional_score(board.black) - get_positional_score(board.white);
    const int corners = (__builtin_popcountll(board.black & CORNERS) - 
                        __builtin_popcountll(board.white & CORNERS)) * Weights<P>::corner;
    const int edges = (edge_stability(board.black) - edge_stability(board.white)) * Weights<P>::edge;
    const int mobility = (calculate_mobility<true>(board) - calculate_mobility<false>(board)) * Weights<P>::mobility;
    const int disc_diff = (__builtin_popcountll(board.black) - __builtin_popcountll(board.white)) * Weights<P>::disc;

    return corners + positional + edges + mobility + disc_diff;
}

int evaluate(const Board& board) {
    const int total_discs = __builtin_popcountll(board.black | board.white);
    
    if (total_discs >= 56) { // Late game
        const int disc_diff = __builtin_popcountll(board.black) - __builtin_popcountll(board.white);
        const int empties = 64 - total_discs;
        const int parity = (empties % 2) * ((disc_diff > 0) ? 50 : -50); // Odd nbr of empties favors the winner
        return disc_diff * 100 + parity + phase_evaluation<LATE_GAME>(board);
    }
    if (total_discs >= 40) { // Mid game
        return phase_evaluation<MID_GAME>(board);
    }
    // Early game
    return phase_evaluation<EARLY_GAME>(board);
}