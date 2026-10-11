### Makefile — libpnm
# Usage:
#   make        compile libpnm.a et l'exécutable pnm
#   make debug  compile avec symboles de debug et sanitizers
#   make clean  supprime les fichiers générés

CC      = gcc
CFLAGS  = -std=c99 -pedantic -Wall -Wextra -O2
AR      = ar
ARFLAGS = rcs

LIB     = libpnm.a
EXEC    = pnm
SRCS    = pnm.c utils.c
OBJS    = $(SRCS:.c=.o)
HEADERS = pnm.h utils.h

.PHONY: all debug clean

all: $(EXEC)

$(EXEC): main.o $(LIB)
	$(CC) $(CFLAGS) $^ -o $@

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS = -std=c99 -pedantic -Wall -Wextra -g -O0 -fsanitize=address,undefined
debug: clean all

clean:
	rm -f main.o $(OBJS) $(LIB) $(EXEC)
