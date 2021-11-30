CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wstrict-aliasing -I include
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
EXEC = cassini

all: $(EXEC)

$(EXEC) : $(OBJ)
	$(CC) -o $@ $(OBJ)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean : 
	rm -rf src/*.o ./$(EXEC)

distclean : 
	rm -rf src/*.o ./$(EXEC)