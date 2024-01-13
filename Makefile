LUA ?= lua5.1
LUA_PC ?= lua5.1
LUA_CFLAGS = $(shell pkg-config $(LUA_PC) --cflags)

CFLAGS ?= -O3 -g -fPIC

SO = lrbtree.so

all: $(SO)

%.o: %.c rbtree.h
	$(CC) -c $(CFLAGS) $(LUA_CFLAGS) -o $@ $<

$(SO): lrbtree.o rbtree.o
	$(CC) -shared $^ -o $@

clean:
	rm -f $(SO) *.o *.rock