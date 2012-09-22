CC=gcc
CFLAGS=-Wall -fPIC -I../alg/ -std=c99 -O2 #-DDEBUG
LDFLAGS=-L./ -L../lib -lm

SOURCE_TOOLS=st_darts.c
OBJS=$(patsubst %.c,%.o,$(SOURCE_TOOLS))

all: target

target: st_tools

st_tools: $(OBJS)
	ar rcs libstiger.a $^
	$(CC) $(CFLAGS) -c st_tools.c -o st_tools.o
	$(CC) $(CFLAGS) st_tools.o -lstiger -lstbasealg $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -r -f -v *.o *.a st_tools

