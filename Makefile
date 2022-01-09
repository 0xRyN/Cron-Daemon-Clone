CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wstrict-aliasing -I include
SRCCASSINI = $(wildcard src/cassini/*.c)
SRCSATURND = $(wildcard src/saturnd/*.c)
SRCCOMMON = $(wildcard src/common/*.c)
OBJCASSINI = $(SRCCASSINI:.c=.o)
OBJSATURND = $(SRCSATURND:.c=.o)
OBJCOMMON = $(SRCCOMMON:.c=.o)
EXEC = cassini saturnd

all: $(EXEC)

cassini : $(OBJCASSINI) $(OBJCOMMON)
	$(CC) -o $@ $(OBJCASSINI) $(OBJCOMMON)

saturnd : $(OBJSATURND) $(OBJCOMMON)
	$(CC) -o $@ $(OBJSATURND) $(OBJCOMMON)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean : 
	rm -f src/*/*.o $(EXEC)

distclean : 
	rm -f src/*/*/.o $(EXEC)

cleanpipe : 
	rm -rf /tmp/rayan/saturnd