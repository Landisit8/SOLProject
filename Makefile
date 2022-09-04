CC		=	gcc
CFLAGS		+=	-std=gnu99 -Wall -Werror -g -pedantic
INCLUDES	=	-I. -I ./include
LDFLAGS		=	-L.
OPTFLAGS 	=
T_LIBS		=	-pthread	

TARGETS 	=	server		\
			client			\
			parsing			\
			lfucache		


.PHONY: all clean cleanall
.SUFFIXES: .c .h

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

all		: $(TARGETS)

bob		: server.o lfucache.o
	$(CC) $(CFLAGS) $(INCLUDES) $(T_LIBS) $(OPTFLAGS) $^ -o $@
server.o:	server.c lfucache.h
lfucache.o: lfucache.c lfucache.h

bau		: client.o API.o
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) $^ -o $@
client.o: client.c API.h
API.o: API.c API.h

clean		: 
	rm -f $(TARGETS)

cleanall	: clean
	\rm -f *.o *~ *.a *.out ./bob ./bau ./canale