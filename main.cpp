#include "board.hpp"
#include "search.hpp"
#include <iostream>
#include <utility>

using std::cin;
using std::cout;

uint64_t convert_to_bitboard(int row, int col) {
    return 1ULL << ((row - 1) * 8 + (col - 1));
}

std::pair<int, int> convert_move(uint64_t move) {
    int square = __builtin_ctzll(move); 
    int row = (square / 8) + 1;
    int col = (square % 8) + 1;
    return {row, col};
}

// bitboard to notation
std::string move_to_notation(uint64_t move) {
    if (move == 0) return "pass";
    
    int pos = __builtin_ctzll(move);
    int row = pos / 8 + 1;
    char col = 'a' + (pos % 8);
    return std::string(1, col) + std::to_string(row);
}

void print_moves(const vector<uint64_t>& moves) {
    for (uint64_t move : moves) {
        int pos = __builtin_ctzll(move); // Get the index of the first set bit
        int r = (pos / 8) + 1;
        int c = (pos % 8) + 1;
        cout << "(" << r << "," << c << ") ";
    }
    cout << endl;
}

// print moves in notation form
void print_moves_notation(const vector<uint64_t>& moves) {
    for (uint64_t move : moves) {
        cout << move_to_notation(move) << " ";
    }
    cout << endl;
}

// notation to bitboard
uint64_t notation_to_move(const std::string& notation) {
    if (notation == "pass") return 0;

    int row = notation[1] - '0';
    int col = notation[0] - 'a';
    return 1ULL << ((row - 1) * 8 + col);
}


int main() {
    Board board;
    int chosen_color;
    bool is_black;

    cout << "Choose color (0=White, 1=Black): ";
    cin >> chosen_color;
    is_black = (chosen_color == 1);

    bool current_player_is_black = true; // Black always starts
    const int TIME_LIMIT_MS = 5000; 
    const int MAX_DEPTH = 60;


    while (!board.is_game_over()) {
        board.print();
        cout << "Current player: " << (current_player_is_black ? "Black" : "White") << endl;

        std::vector<uint64_t> moves = board.get_moves(current_player_is_black);
        if (moves.empty()) {
            cout << "No moves available - passing!\n";
            current_player_is_black = !current_player_is_black;
            continue;
        }

        bool human_turn = (current_player_is_black == is_black);

        if (human_turn) {
            // cout << "Your move (TWO space separated digits) row col : ";
            cout << "Your move (in the form of a1, b2, etc.): ";
            cout << "\nAvailable moves: ";
            // print_moves(moves);
            print_moves_notation(moves);

            // int row, col;
            // cin >> row >> col;
            // uint64_t move = convert_to_bitboard(row, col);

            std::string move_notation;
            cin >> move_notation;
            uint64_t move = notation_to_move(move_notation);

            if (std::find(moves.begin(), moves.end(), move) != moves.end()) {
                board.make_move(move, current_player_is_black);
            } else {
                cout << "Invalid move! Try again.\n";
                continue;
            }
        } else {
            cout << "AI is processing...\n";
            Search searcher;
            SearchResult result = searcher.iterative_deepening(
                board, current_player_is_black, TIME_LIMIT_MS, MAX_DEPTH
            );

            board.make_move(result.move, current_player_is_black);
            cout << "AI played: ";
            cout << move_to_notation(result.move) << "\n";            
            // auto [row, col] = convert_move(result.move);
            // cout << "(" << row << ", " << col << ")";
            // cout << endl;
            if (result.depth > 0) cout << "AI searched to depth " << result.depth << "\n";
        }
        current_player_is_black = !current_player_is_black;
    }

    board.print();
    int black_count = __builtin_popcountll(board.black);
    int white_count = __builtin_popcountll(board.white);

    cout << "Game over!\n";
    cout << "\nBlack count: " << black_count << "\nWhite count: " << white_count << "\n";
    if (black_count > white_count) cout << "Black wins!\n"; 
    else if (white_count > black_count) cout << "White wins!\n"; 
    else cout << "It's a draw!\n";

    return 0;
}
