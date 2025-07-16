#pragma once

#include "board.hpp"
#include "evaluation.hpp"
#include "transPositionTable.hpp"
#include "zobrist.hpp"
#include <chrono>
#include <algorithm>
#include <climits>
#include <cstdint>
#include <sys/types.h>


#define DEBUG		0
#if DEBUG
#define pr(...)  do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#else 	//str(%s), ll(%lld), char(%c), double/float(%f or %0.6f)
#define pr(...)
#endif


using namespace std::chrono;
const int INF = 1e8;
using std::cout;
using std::endl;

struct SearchResult {
    uint64_t move = 0;
    int value = 0;
    int depth = 0;
};


class Search {
    TranspositionTable tt;
    steady_clock::time_point start_time;
    int time_limit;
    bool timeout = false;

public:
    Search() {}

    SearchResult iterative_deepening(Board& board, bool is_black, int time_ms, int max_depth) {
        start_time = steady_clock::now();
        time_limit = time_ms;
        timeout = false;
        tt.new_search();

        SearchResult best_result;
        uint64_t board_hash = Zobrist::compute_hash(board, is_black);

        for (int depth = 1; depth <= max_depth; ++depth) {
            int alpha = -INF;
            int beta = INF;

            SearchResult current = alpha_beta(board, board_hash, depth, alpha, beta, is_black);

            if (timeout) break;

            best_result = current;
            best_result.depth = depth;

            // Early exit if game is decided
            if(abs(current.value) > INF/2) break;
        }
        return best_result;
    }

private:
    SearchResult alpha_beta(Board& board, uint64_t hash, int depth, int alpha, int beta, bool is_black_turn) {
        if (check_timeout()) return {0, 0, depth};

        pr("Entering alpha_beta\n");

        // TT Lookup
        int tt_alpha = alpha, tt_beta = beta;
        int tt_value;
        uint64_t tt_move = 0;

        pr("Before probing TT\n");
        if (tt.probe(hash, depth, tt_alpha, tt_beta, tt_value, tt_move)) {
            pr("TT Hit\n");
            return {tt_move, tt_value, depth};
        }

        pr("Before getting moves\n");
        vector<uint64_t> moves = board.get_moves(is_black_turn);
        pr("Got moves %zu\n", moves.size());
        if (moves.empty()) {
            return {0, evaluate(board), depth};
        }

        // Move ordering: TT move first
        if (tt_move && find(moves.begin(), moves.end(), tt_move) != moves.end()) {
            std::swap(moves[0], *find(moves.begin(), moves.end(), tt_move));
        }

        SearchResult best_result;
        best_result.value = is_black_turn ? -INF : INF;
        EntryType tt_type = EntryType::UPPERBOUND;

        for(auto move : moves) {
            pr("Entering move loop\n");
            if (check_timeout()) return best_result;

            Board new_board = board;
            new_board.make_move(move, is_black_turn);

            int square = __builtin_ctzll(move); // Count trailing zeros
            uint64_t new_hash = hash ^ Zobrist::zobrist_data.squares[square][is_black_turn ? 0 : 1];
            new_hash ^= Zobrist::zobrist_data.black_to_move;

            SearchResult current;
            if (depth == 1) {
                current.value = evaluate(new_board);
            } else {
                current = alpha_beta(new_board, new_hash, depth - 1, alpha, beta, !is_black_turn);
            }

            if (is_black_turn) {
                if (current.value > best_result.value) {
                    best_result.value = current.value;
                    best_result.move = move;
                    alpha = std::max(alpha, best_result.value);
                }
            } else {
                if (current.value < best_result.value) {
                    best_result.value = current.value;
                    best_result.move = move;
                    beta = std::min(beta, best_result.value);
                }
            }

            if (alpha >= beta) {
                tt_type = EntryType::LOWERBOUND;
                break;
            }
        }

        // Entry type?
        if (best_result.value <= tt_alpha) tt_type = EntryType::UPPERBOUND;
        else if (best_result.value >= tt_beta) tt_type = EntryType::LOWERBOUND;
        else tt_type = EntryType::EXACT;

        tt.store(hash, depth, best_result.value, tt_type, best_result.move);
        return best_result;
    }

    bool check_timeout() {
        if (!timeout && duration_cast<milliseconds>(steady_clock::now() - start_time).count() > time_limit) {
            timeout = true;
        }
        return timeout;
    }
};


