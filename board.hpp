#pragma once

#include <sys/types.h>
#include <vector>
#include <iostream>
#include <cstdint> 

using std::vector;
using std::cout;
using std::endl;
const int BOARD_SIZE = 8;

struct Board {
    uint64_t black;
    uint64_t white;

    /*
        black: (4, 3) and (3, 4)
        conversion: row * 8 + col
        (4, 3) = 4*8+3 = 35
        (3, 4) = 3*8+4 = 28

        white: (3, 3) and (4, 4)
        conversion: row* 8 + col
        (3, 3) = 3*8+3 = 27
        (4, 4) = 4*8+4 = 36
    */
    Board(): black(0), white(0) {
        black |= (1ULL << (4 * 8 + 3));
        black |= (1ULL << (3 * 8 + 4));
        white |= (1ULL << (3 * 8 + 3));
        white |= (1ULL << (4 * 8 + 4));
    }


    void print() const {
        cout << "  a b c d e f g h\n";
        for (int i = 0; i < BOARD_SIZE; ++i) {
            cout << (i + 1) << " ";
            for (int j = 0; j < BOARD_SIZE; ++j) {
                uint64_t mask = 1ULL << (i * BOARD_SIZE + j);
                if (white & mask) cout << "\033[1;31mw\033[0m ";
                else if (black & mask) cout << "\033[1;34mb\033[0m ";
                else cout << ". ";
            }
            cout << (i + 1) << endl;
        }
        cout << "  1 2 3 4 5 6 7 8 \n\n";
    }


    // get all possible moves for a player
    vector<uint64_t> get_moves(bool is_black) const {
        uint64_t player = is_black ? black : white;
        uint64_t opponent = is_black ? white : black;

        vector<uint64_t> moves;
        uint64_t empty = ~(player | opponent);

        for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
            uint64_t mask = 1ULL << i;
            if (empty & mask && is_valid_move(mask, is_black)) {
                moves.push_back(mask);
            }
        }
        return moves;
    }

    static uint64_t shift(uint64_t mask, int dir) {
        switch (dir) {
            // Fixed directions
            case 0: return (mask >> 8) & 0x00FFFFFFFFFFFFFF; // Up 
            case 1: return (mask << 8) & 0xFFFFFFFFFFFFFF00; // Down 
            case 2: return (mask & 0x7F7F7F7F7F7F7F7F) << 1;  // Right
            case 3: return (mask & 0xFEFEFEFEFEFEFEFE) >> 1;  // Left
            case 4: return (mask & 0xFEFEFEFEFEFEFEFE) >> 9;  // Up-Left
            case 5: return (mask & 0x7F7F7F7F7F7F7F7F) >> 7;  // Up-Right
            case 6: return (mask & 0xFEFEFEFEFEFEFEFE) << 7;  // Down-Left
            case 7: return (mask & 0x7F7F7F7F7F7F7F7F) << 9;  // Down-Right
            default: return 0;
        }
    }


    bool is_valid_move(uint64_t move, bool is_black) const {
        uint64_t player = is_black ? black : white;
        uint64_t opponent = is_black ? white : black;

        for (int dir = 0; dir < BOARD_SIZE; ++dir) {
            uint64_t mask = move;
            uint64_t flipped = 0;
            mask = shift(mask, dir);

            while(mask && (mask & opponent)) {
                flipped |= mask;
                mask = shift(mask, dir);
            }

            if ((mask & player) && flipped) {
                return true;
            }
        }
        return false;
    }


    void make_move(uint64_t move, bool is_black) {
        uint64_t player = is_black ? black : white;
        uint64_t opponent = is_black ? white : black;
        uint64_t to_flip = 0;

        for (int dir = 0; dir < 8; ++dir) {
            uint64_t temp_flip = 0;
            uint64_t cursor = shift(move, dir);
            
            while (cursor && (cursor & opponent)) {
                temp_flip |= cursor;
                cursor = shift(cursor, dir);
            }
            
            if (cursor & player) {
                to_flip |= temp_flip;
            }
        }

        player ^= move | to_flip;
        opponent ^= to_flip;

        if (is_black) {
            black = player;
            white = opponent;
        } else {
            white = player;
            black = opponent;
        }
    }

    bool is_game_over() const {
        return get_moves(true).empty() && get_moves(false).empty();
    }

};
