CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c99 -Isrc
SRCS = src/main.c src/bitboard.c src/position.c src/move.c src/movegen.c src/makemove.c src/eval.c src/tt.c src/search.c src/perft.c src/uci.c src/cli.c
TARGET = chess_engine.exe

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	del /f /q $(TARGET) src\*.o 2>NUL || rm -f $(TARGET) src/*.o
