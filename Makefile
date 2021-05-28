CC		=	gcc
CFLAGS		+=	-std=c99 -Wall -Werror -g -pedantic
INCLUDES	=	-I. -I ./include
LDFLAGS		=	-L.
OPTFLAGS 	=
LIBS		=	-lpthread	

TARGETS 	=	server		\
			client			\
			parsing			\
			lfucache		


.PHONY: all clean cleanall
.SUFFIXES: .c .h

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

all		: $(TARGETS)

bob		: server.o parsing.o lfucache.o
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) $^ -o $@
server.o:	server.c ./include/parsing.h

parsing.o: parsing.c ./include/parsing.h
lfucache.o: lfucache.c ./include/lfucache.h

bau		: client.o
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) $^ -o $@
client.o: client.c

clean		: 
	rm -f $(TARGETS)

cleanall	: clean
	\rm -f *.o *~ *.a *.out ./bob ./canale 