@echo off
echo Building Chess Engine in C using MinGW gcc...
gcc -O3 -Wall -Wextra -std=c99 -Isrc src/main.c src/bitboard.c src/position.c src/move.c src/movegen.c src/makemove.c src/eval.c src/tt.c src/search.c src/perft.c src/uci.c src/cli.c -o chess_engine.exe
if %ERRORLEVEL% EQU 0 (
    echo.
    echo BUILD SUCCESSFUL! Executable created: chess_engine.exe
) else (
    echo.
    echo BUILD FAILED with error code %ERRORLEVEL%.
)
