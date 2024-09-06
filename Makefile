CC := musl-gcc
CFLAGS := -Wall -static

all: binhex

clean:
	rm binhex
