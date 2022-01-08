CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wstrict-aliasing -I include
SRCCASSINI = $(wildcard src/cassini/*.c)
SRCSATURND = $(wildcard src/saturnd/*.c)
OBJCASSINI = $(SRCCASSINI:.c=.o)
OBJSATURND = $(SRCSATURND:.c=.o)
EXEC = cassini saturnd

all: $(EXEC)

cassini : $(OBJCASSINI)
	$(CC) -o $@ $(OBJCASSINI)

saturnd : $(OBJSATURND)
	$(CC) -o $@ $(OBJSATURND)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean : 
	rm -f src/*/*.o $(EXEC)

distclean : 
	rm -f src/*/*/.o $(EXEC)