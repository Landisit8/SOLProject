CC		=	gcc
CFLAGS		+=	-std=gnu99 -Wall -Werror -g -pedantic
INCLUDES	=	-I. -I ./include
LDFLAGS		=	-L.
OPTFLAGS 	=
T_LIBS		=	-pthread	

TARGETS 	=	server		\
			client		


.PHONY: all clean cleanall
.SUFFIXES: .c .h

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

all		: $(TARGETS)

server: server.c libs/libtree.so libs/libapi.so
	$(CC) $(CFLAGS) $(INCLUDES) $(T_LIBS) server.c -o server -Wl,-rpath,./libs -L ./libs -ltree -lapi

client: client.c libs/libtree.so libs/libapi.so
	$(CC) $(CFLAGS) $(INCLUDES) client.c -o client -Wl,-rpath,./libs -L ./libs -lapi


libs/libtree.so: lfucache.o
	$(CC) -shared -o libs/libtree.so $^
lfucache.o:
	$(CC) $(CFLAGS) $(INCLUDES) lfucache.c -c -fPIC -o $@

libs/libapi.so: API.o
	$(CC) -shared -o libs/libapi.so $^
API.o:
	$(CC) $(CFLAGS) $(INCLUDES) API.c -c -fPIC -o $@

clean		: 
	rm -f $(TARGETS)

cleanall	: clean
	\rm -f *.o *~ *.a *.out ./server ./client ./canale ./logFile.txt ./libs/*.* ./read/*.* ./LFU/*.*

test1: all
	./test/test1.sh
test2: all
	./test/test2.sh