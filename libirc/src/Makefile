# Pick a collection type for callbacks. 
# AVL Trees are efficient in terms of memory but a little slower
CALLBACK=-DCALLBACK_COLLECTION_IS_AVLTREE

# Hashtables are less efficient in terms of memory, but a lot faster
#CALLBACK=-DCALLBACK_COLLECTION_IS_HASHTABLE

# Uncomment this if you need tracing support 
#TRACE=-DENABLE_TRACE

# If you need AVL trees in your own code, uncomment this line
#AVLTREE=USE_AVLTREE

# If you need hash table in your own code, uncomment this line
#HASHTABLE=-DUSE_HASHTABLE

# If you want to compile this library in "release mode" uncomment this
COMPILE_FLAGS=-DNDEBUG -O2

# If you want to compile this library in "debug mode" uncomment this
#COMPILE_FLAGS=-g

# Uncomment this if you want to process CTCP messages yourself
#CTCP=-DDONT_PROCESS_CTCP

####### No editing is needed below this line ###########

DEFS=$(CALLBACK) $(TRACE) $(AVLTREE) $(HASHTABLE) $(CTCP)
CFLAGS=$(COMPILE_FLAGS) -Wall -fPIC -shared $(DEFS)
CC=gcc
OBJS=ctcp.o irc_core.o irc_funcs.o tcp.o avltree.o map.o list.o \
hashtable.o trace.o bufsock.o
LDFLAGS=-shared

all: $(OBJS)
	$(CC) -o libirc.so $(OBJS) $(LDFLAGS)
	$(AR) cru libirc.a $(OBJS)

clean:
	rm -f $(OBJS) libirc.so libirc.a

distclean: clean
	rm -rf *~
