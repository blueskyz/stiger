CC=gcc
CFLAGS=-Wall -std=c99 -O2 # -DDEBUG
LDFLAGS=-liconv

SOURCE_TOOLS=st_tools.c st_darray.c st_darts.c st_utils.c
OBJS=$(patsubst %.c,%.o,$(SOURCE_TOOLS))

all: target

target: st_tools

st_tools: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -r -f -v *.o

#	gcc -std=c99 st_tools.c st_darray.c st_darts.c st_utils.c -liconv -o st_tools