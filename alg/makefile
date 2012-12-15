CC=gcc
#CFLAGS=-g -Wall -fPIC -std=c99 -I../ -O0 -DDEBUG
CFLAGS=-g -Wall -fPIC -std=c99 -I../ -O2
#LDFLAGS=-L./lib -lm -liconv
LDFLAGS=-L./lib -lm

SOURCE_TOOLS= st_heap.c st_huffman_s.c st_darray.c st_utils.c
OBJS=$(patsubst %.c,%.o,$(SOURCE_TOOLS))
LIBOBJS=$(patsubst %.c,obj/%.o,$(SOURCE_TOOLS))

all: target

target: ready st_test

ready:
	test -d obj || mkdir -pv obj
	test -d lib || mkdir -pv lib
	test -d ../lib || mkdir -pv ../lib

st_test: $(OBJS)
	ar rcs lib/libstbasealg.a $(LIBOBJS)
	cp -v -f lib/* ../lib/
	$(CC) $(CFLAGS) -c st_test.c -o obj/st_test.o
	$(CC) $(CFLAGS) obj/st_test.o -lstbasealg $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o obj/$@

clean:
	rm -r -f -v obj lib st_test
