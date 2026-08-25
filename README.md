# C Chess Engine

A fast, modular, high-performance Chess Engine written in C, featuring Bitboard board representation, Negamax Alpha-Beta search with Transposition Tables, PeSTO tapered evaluation, Universal Chess Interface (UCI) protocol compliance, and an interactive command-line interface.

## Key Features

- **64-bit Bitboard Architecture**: Precalculated attack bitboard tables for Pawns, Knights, Kings, and sliding piece ray attack generators.
- **Negamax Alpha-Beta Search**: Quiescence search, Null Move Pruning, Late Move Reductions (LMR), MVV-LVA capture ordering, Killer move heuristic, and History heuristic.
- **Transposition Table**: 64-bit Zobrist position hashing with exact, lower-bound, and upper-bound score flags.
- **PeSTO Tapered Evaluation**: Dynamic phase-blended evaluation (Middlegame & Endgame) for positional understanding.
- **Dual Mode Support**:
  - **UCI Mode**: Standard Universal Chess Interface for external GUIs (Arena, CuteChess, Lichess).
  - **Interactive CLI Mode**: Visual terminal board renderer, human vs engine, self-play simulation (`auto`), FEN loader, and undo.
- **Perft Test Suite**: Built-in move generation correctness verifier against standard benchmark positions.

## Build Instructions

### Windows (MinGW GCC)
```cmd
.\build.bat
```

### Linux / macOS (GCC or Clang)
```bash
make
```

## Running the Engine

- **Launch Interactive CLI**:
  ```cmd
  .\chess_engine.exe
  ```

- **Launch UCI Mode**:
  ```cmd
  .\chess_engine.exe uci
  ```

- **Run Perft Test Suite**:
  ```cmd
  .\chess_engine.exe perft
  ```

## Project Structure

```text
├── src/
│   ├── main.c        # Program entry point & CLI flag parsing
│   ├── defs.h        # Constants, data structures, and move macros
│   ├── bitboard.c/h  # 64-bit bitboard operations & attack lookup tables
│   ├── position.c/h  # Board state, Zobrist hashing, FEN parser/generator
│   ├── move.c/h      # Move string parsing & formatting
│   ├── movegen.c/h   # Move generator & attack checking
│   ├── makemove.c/h  # Move execution, undo, & castling/ep state stack
│   ├── eval.c/h      # PeSTO tapered evaluation function
│   ├── tt.c/h        # Transposition Table allocation & probe/write
│   ├── search.c/h    # Negamax search, alpha-beta pruning, quiescence
│   ├── perft.c/h     # Perft move generator tester
│   ├── uci.c/h       # UCI protocol handler
│   └── cli.c/h       # Interactive CLI mode & ASCII board renderer
├── Makefile          # GNU Make build configuration
├── build.bat         # Windows build script
└── README.md         # Documentation
```

## License
MIT License
