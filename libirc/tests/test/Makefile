CC=gcc
DEFS=-D__USE_POSIX
CFLAGS=-g -Wall $(DEFS) -I../src
#CFLAGS=-O2 -Wall $(DEFS) -I../src
LDFLAGS=../src/libirc.a
OBJS=main.o

all: $(OBJS)
	$(CC) -o irctst $(OBJS) $(LDFLAGS)

compile_only: $(OBJS)

clean: 
	rm -f $(OBJS)

distclean: clean
	rm -f *~ irctst 
