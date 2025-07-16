// gui.cpp
#include "board.hpp"
#include "search.hpp"
#include <raylib.h>
#include <future>
#include <thread>
#include <atomic>


const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;
const int CELL_SIZE = SCREEN_HEIGHT / 8;
const int BOARD_OFFSET_X = (SCREEN_WIDTH - SCREEN_HEIGHT) / 2;
const int TIME_LIMIT_MS = 5000; 
const int MAX_DEPTH = 60;

// Colors
const Color DARK_GREEN = {34, 139, 34, 255};
const Color LIGHT_GREEN = {144, 238, 144, 255};
const Color BLACK_PIECE = {25, 25, 25, 255};
const Color WHITE_PIECE = {230, 230, 230, 255};
const Color LEGAL_MOVE_COLOR = {255, 255, 0, 100};

struct GameState {
    Board board;
    bool human_is_black;
    bool current_player_black = true;
    std::vector<uint64_t> current_moves;
    std::atomic<bool> ai_thinking{false};
    std::future<SearchResult> ai_result;
    uint64_t last_ai_move = 0;
};

void DrawBoard(const GameState& state) {
    // Draw board background
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Color cell_color = (row + col) % 2 == 0 ? DARK_GREEN : LIGHT_GREEN;
            DrawRectangle(BOARD_OFFSET_X + col * CELL_SIZE, row * CELL_SIZE, 
                         CELL_SIZE, CELL_SIZE, cell_color);
        }
    }

    // Draw pieces
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            uint64_t pos = 1ULL << (row * 8 + col);
            int x = BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE/2;
            int y = row * CELL_SIZE + CELL_SIZE/2;

            if (state.board.black & pos) {
                DrawCircle(x, y, CELL_SIZE/2 - 5, BLACK_PIECE);
            }
            else if (state.board.white & pos) {
                DrawCircle(x, y, CELL_SIZE/2 - 5, WHITE_PIECE);
            }
        }
    }

    // Draw legal moves
    for (auto move : state.current_moves) {
        int pos = __builtin_ctzll(move);
        int row = pos / 8;
        int col = pos % 8;
        DrawCircle(BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE/2,
                  row * CELL_SIZE + CELL_SIZE/2, 10, LEGAL_MOVE_COLOR);
    }

    // Draw AI last move
    if (state.last_ai_move) {
        int pos = __builtin_ctzll(state.last_ai_move);
        int row = pos / 8;
        int col = pos % 8;
        DrawRectangleLines(BOARD_OFFSET_X + col * CELL_SIZE, row * CELL_SIZE,
                          CELL_SIZE, CELL_SIZE, RED);
    }
}

void RunGUI(GameState& state) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Othello");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Human move handling
        if (!state.ai_thinking && state.current_player_black == state.human_is_black) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                int col = (mousePos.x - BOARD_OFFSET_X) / CELL_SIZE;
                int row = mousePos.y / CELL_SIZE;

                if (row >= 0 && row < 8 && col >= 0 && col < 8) {
                    uint64_t move = 1ULL << (row * 8 + col);
                    if (std::find(state.current_moves.begin(), state.current_moves.end(), move) != state.current_moves.end()) {
                        state.board.make_move(move, state.current_player_black);
                        state.current_player_black = !state.current_player_black;
                        state.last_ai_move = 0;
                    }
                }
            }
        }

        // AI move handling
        if (!state.ai_thinking && state.current_player_black != state.human_is_black) {
            state.ai_thinking = true;
            state.ai_result = std::async(std::launch::async, [&state]() {
                Board ai_board = state.board;
                Search searcher;
                return searcher.iterative_deepening(ai_board, state.current_player_black, TIME_LIMIT_MS, MAX_DEPTH);
            });
        }

        // Check AI result
        if (state.ai_thinking && state.ai_result.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            SearchResult result = state.ai_result.get();
            state.board.make_move(result.move, state.current_player_black);
            state.last_ai_move = result.move;
            state.current_player_black = !state.current_player_black;
            state.ai_thinking = false;
        }

        // Update legal moves
        state.current_moves = state.board.get_moves(state.current_player_black);
        if (state.current_moves.empty() && !state.board.is_game_over()) {
            state.current_player_black = !state.current_player_black;
        }

        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawBoard(state);
        
        if (state.ai_thinking) {
            DrawText("AI is thinking...", 10, 10, 20, DARKGRAY);
        }
        
        if (state.board.is_game_over()) {
            int black = __builtin_popcountll(state.board.black);
            int white = __builtin_popcountll(state.board.white);
            const char* text = black > white ? "Black wins!" : white > black ? "White wins!" : "Draw!";
            DrawText(text, SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2 - 10, 20, BLACK);
        }

        EndDrawing();
    }

    CloseWindow();
}

int main() {
    GameState state;
    
    // Color selection window
    InitWindow(350, 150, "Choose Color");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GREEN);

        DrawText("Choose color (Click left/right)", 20, 20, 20, BLACK);
        DrawText("Black", 50, 80, 30, BLACK);
        DrawText("White", 200, 80, 30, WHITE);
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            if (mousePos.x < 150) {
                state.human_is_black = true;
                break;
            }
            else {
                state.human_is_black = false;
                break;
            }
        }
        EndDrawing();
    }
    CloseWindow();

    RunGUI(state);
    return 0;
}