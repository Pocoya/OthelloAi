#pragma once


#include "board.hpp"
#include <array>
#include <cstdint>
#include <sys/types.h>
#include <random>

namespace Zobrist {
    struct Lehmer64 {
        uint64_t state;
        constexpr Lehmer64(uint64_t seed) : state(seed) {}
        constexpr uint64_t next() {
            state = (state * 0xda942042e4dd58b5ULL); // 64-bit Lehmer multiplicative PRNG
            return state;
        }
    };

    inline auto generate_zobrist_data() {
        struct {
            std::array<std::array<uint64_t, 2>, 64> squares;
            uint64_t black_to_move;
        } data;

        static_assert(sizeof(uint64_t) == 8, "Zobrist requires 64-bit system");
        std::mt19937_64 rng(0xDEADBEEF);  // Fixed seed for reproducibility
        
        for (auto& square : data.squares) {
            square[0] = rng();  // Black pieces
            square[1] = rng();  // White pieces
        }
        data.black_to_move = rng();
        return data;
    }


    inline const auto zobrist_data = generate_zobrist_data();
    inline const auto& zobrist_table = zobrist_data.squares;
    inline const auto black_to_move_key = zobrist_data.black_to_move;


    inline uint64_t compute_hash(const Board& board, bool is_black_turn) {
        uint64_t hash = 0;
        // black
        uint64_t black = board.black;
        while(black) {
            int idx = __builtin_ctzll(black);  // Count trailing zeros (iterate over set bits)
            hash ^= zobrist_table[idx][0];
            black ^= (1ULL << idx);
        }

        // white
        uint64_t white = board.white;
        while(white) {
            int idx = __builtin_ctzll(white);
            hash ^= zobrist_table[idx][1];
            white ^= (1ULL << idx);
        }

        // side to move
        if (is_black_turn) {
            hash ^= black_to_move_key;
        }
        return hash;
    }
}