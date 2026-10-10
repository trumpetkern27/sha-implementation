CC = clang
CFLAGS = -O0 -std=c23 -g -gdwarf-4

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC = $(call rwildcard,src,*.c,*.h)

all: sha-1

sha-1: $(SRC)
	$(CC) $(CFLAGS) -o sha-1 $(SRC)

clean:
	rm -f sha-1
