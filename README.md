# Othello-AI Engine

An AI-powered **Othello** (Reversi) game engine implemented with **Alpha-Beta Pruning**
## Key Features
- **Iterative Deppening**: The search depth increases iteratively until allotted time expires. The time limit for each move can be configured in main.
- **Transposition Table**: Board states are cached as they are evaluated, in case the same position is encountered again.
- **Zobrist Hashing**: Provides an efficient and unique representation of board states for fast lookup in the transposition table.


## How to Run
### Terminal Version
```bash
make run_othello
```

### GUI Version
```bash
make run_gui
```
