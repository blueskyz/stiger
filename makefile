CC=gcc
CFLAGS=-Wall -std=c99 -O2 #-DDEBUG
LDFLAGS=-L./ -lm

SOURCE_TOOLS=st_darray.c st_darts.c st_utils.c
OBJS=$(patsubst %.c,%.o,$(SOURCE_TOOLS))

all: target

target: st_tools

st_tools: $(OBJS)
	ar rcs libstiger.a $^
	$(CC) $(CFLAGS) -c st_tools.c -o st_tools.o
	$(CC) $(CFLAGS) st_tools.o $(LDFLAGS) -lstiger -lm -o $@

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -r -f -v *.o *.a st_tools

#	gcc -std=c99 st_tools.c st_darray.c st_darts.c st_utils.c -liconv -o st_tools
