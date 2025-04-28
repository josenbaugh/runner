CC = clang
CFLAGS = -std=c99 -Wall -Werror
LDFLAGS = -lncursesw
SRC = runner.c
OUT = runner

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)
